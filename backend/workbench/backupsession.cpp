#include "backupsession.h"
#include <QJsonDocument>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStorageInfo>
#include <QTimer>
#include <limits>
#include <stdexcept>
namespace {
const QStringList roots{QStringLiteral("/int"),QStringLiteral("/ext")};
constexpr int MaximumEntries = 200000;
bool validChild(const QString &parent, const QString &path) {
    if (!path.startsWith(parent + '/') || path.size() > 4096 || path.contains(QChar(0))) return false;
    const auto name = path.mid(parent.size()+1);
    return !name.isEmpty() && name != "." && name != ".." && !name.contains('/');
}
}
class BackupSession::Sink : public QIODevice {
 public:
    explicit Sink(BackupSession &session) : QIODevice(&session), session_(session) {}
    bool isSequential() const override { return true; }
 protected:
    qint64 readData(char *, qint64) override { return -1; }
    qint64 writeData(const char *data, qint64 size) override { return session_.write(data,size); }
 private:
    BackupSession &session_;
};
BackupSession::BackupSession(QString root,QObject *parent) : QObject(parent),root_(std::move(root)),sink_(new Sink(*this)) {
    // Reclassify abandoned writing entries at startup, before the next backup.
    if(QFileInfo::exists(root_+"/backups/catalog.sqlite")) {
        try {UFlipper::BackupStore recovery(root_.toStdString());}
        catch(const std::exception &e){error_=QString::fromUtf8(e.what());state_="interrupted";}
    }
}
BackupSession::~BackupSession() {
    if (verification_) { verification_->wait(); delete verification_; verification_=nullptr; }
    if (active_ && store_) {try {store_->interrupt(id_.toStdString(),"Application closed during full backup");}catch(...) {}}
}
void BackupSession::setConnection(const QString &identity,bool online) {
    if(identity==identity_ && online==online_)return;
    const bool replacement=online && (!online_ || identity!=identity_);
    if(active_ && state_!="verifying" && (!online || replacement)) {
        if(error_.isEmpty())error_=cancelled_?tr("Sicherung abgebrochen; Geräteverbindung geschlossen."):tr("Geräteverbindung unterbrochen.");
        online_=false;
        jobs_.disconnectDevice(error_.toStdString());finalize(false,true);
    }
    if(replacement) {identity_=identity;jobs_.connectDevice(identity.toStdString());uncertain_=false;}
    online_=online;emit changed();
}
bool BackupSession::begin(const QVariantMap &metadata) {
    if(!available())return false;
    error_.clear();id_.clear();job_.clear();inventory_.clear();finalInventory_.clear();digests_.clear();files_.clear();directories_.clear();
    bytesDone_=bytesTotal_=currentBytes_=0;filesDone_=rootIndex_=digestIndex_=0;
    cancelled_=verified_=finalPass_=false;
    try {
        store_=std::make_shared<UFlipper::BackupStore>(root_.toStdString());
        id_=QString::fromStdString(store_->begin(UFlipper::BackupScope::FullDevice,QJsonDocument::fromVariant(metadata).toJson(QJsonDocument::Compact).toStdString()));
        auto request=jobs_.request("backup.full-device",UFlipper::Control|UFlipper::Storage,UFlipper::Origin::Ui);
        if(request.decision!=UFlipper::Decision::Granted)throw std::runtime_error("Device resources are unavailable");
        job_=QString::fromStdString(request.id);jobs_.markRunning(request.id);active_=true;state_="inventory";
        paint_.start();issue(StorageInfo,roots[0]);return true;
    } catch(const std::exception &e) {error_=QString::fromUtf8(e.what());finalize(false,true);return false;}
}
void BackupSession::issue(int command,const QString &path) {
    if(!active_)return;
    pending_=command;current_=path;++token_;emit changed();emit commandRequested(command,path,token_);
}
bool BackupSession::accepts(quint64 token) const {return active_ && !cancelled_ && token==token_;}
void BackupSession::storageInfo(quint64 token,bool present) {
    if(!accepts(token) || pending_!=StorageInfo)return;
    if(!present) {finishError(tr("Speicher %1 ist nicht verfügbar. Kein vollständiges Backup.").arg(current_),true);return;}
    if(++rootIndex_<roots.size())issue(StorageInfo,roots[rootIndex_]);
    else {rootIndex_=0;for(const auto &root:roots)directories_.enqueue(root);nextDirectory();}
}
void BackupSession::nextDirectory() {
    if(!active_ || cancelled_)return;
    if(!directories_.isEmpty()) {issue(List,directories_.dequeue());return;}
    try {
        if(finalPass_) {
            if(finalInventory_.size()!=inventory_.size())throw std::runtime_error("Device inventory changed during backup");
            for(auto it=inventory_.cbegin();it!=inventory_.cend();++it) {
                const auto found=finalInventory_.constFind(it.key());
                if(found==finalInventory_.cend() || found->directory!=it->directory || found->bytes!=it->bytes)
                    throw std::runtime_error("Device inventory changed during backup");
            }
            state_="checking-device";digestIndex_=0;nextDigest();
        } else {
            // Fail before file transfer when local capacity is already insufficient.
            const QStorageInfo storage(root_);
            if(storage.isValid() && storage.bytesAvailable()>=0 && quint64(storage.bytesAvailable())<bytesTotal_+16*1024*1024)
                throw std::runtime_error("Insufficient local disk space for the complete device backup");
            state_="copying";nextFile();
        }
    } catch(const std::exception &e) {finishError(QString::fromUtf8(e.what()),true);}
}
void BackupSession::listed(quint64 token,const QList<BackupRemoteEntry> &entries) {
    if(!accepts(token) || pending_!=List)return;
    try {
        auto &map=finalPass_?finalInventory_:inventory_;
        if(map.contains(current_))throw std::runtime_error("Duplicate backup directory");
        map.insert(current_,{current_,0,true});
        if(!finalPass_)store_->addDirectory(id_.toStdString(),current_.toStdString());
        for(const auto &entry:entries) {
            if(!validChild(current_,entry.path) || map.contains(entry.path) || directories_.contains(entry.path))
                throw std::runtime_error("Unsafe, duplicate or inconsistent device inventory");
            if(entry.directory)directories_.enqueue(entry.path);
            else {
                map.insert(entry.path,entry);
                if(!finalPass_) {
                    if(entry.bytes>quint64(std::numeric_limits<qint64>::max())-bytesTotal_)throw std::runtime_error("Backup size exceeds supported range");
                    bytesTotal_+=entry.bytes;files_.append(entry.path);
                }
            }
            if(map.size()+directories_.size()>MaximumEntries)throw std::runtime_error("Backup inventory exceeds the 200000 entry limit");
        }
        nextDirectory();
    } catch(const std::exception &e){finishError(QString::fromUtf8(e.what()),true);}
}
void BackupSession::nextFile() {
    if(!active_ || cancelled_)return;
    try {
        if(filesDone_==files_.size()) {
            finalPass_=true;state_="checking-inventory";for(const auto &root:roots)directories_.enqueue(root);nextDirectory();return;
        }
        const auto &path=files_[filesDone_];
        store_->beginFile(id_.toStdString(),path.toStdString(),inventory_[path].bytes);
        currentBytes_=0;localMd5_.reset();issue(Read,path);
    } catch(const std::exception &e){finishError(QString::fromUtf8(e.what()),true);}
}
QIODevice *BackupSession::sink() const {return sink_.get();}
qint64 BackupSession::write(const char *data,qint64 size) {
    if(size<0 || !active_ || pending_!=Read)return -1;
    if(cancelled_)return size;
    try {
        store_->appendFile(id_.toStdString(),data,size_t(size));
        localMd5_.addData(data,size);currentBytes_+=quint64(size);bytesDone_+=quint64(size);
        if(paint_.elapsed()>80) {paint_.restart();emit changed();}
        return size;
    } catch(const std::exception &e) {
        error_=QString::fromUtf8(e.what());cancelled_=true;state_="stopping";emit changed();
        QTimer::singleShot(0,this,[this]{emit transportStopRequested();});return size;
    }
}
void BackupSession::readFinished(quint64 token) {
    if(!accepts(token) || pending_!=Read)return;
    try {
        if(currentBytes_!=inventory_[current_].bytes)throw std::runtime_error("Remote file size changed during backup");
        store_->finishFile(id_.toStdString());digests_.insert(current_,localMd5_.result().toHex());issue(Digest,current_);
    } catch(const std::exception &e){finishError(QString::fromUtf8(e.what()),true);}
}
void BackupSession::digested(quint64 token,const QByteArray &sum) {
    if(!accepts(token) || pending_!=Digest)return;
    const auto md5=sum.trimmed().toLower();
    if(md5.size()!=32 || md5!=digests_.value(current_)) {finishError(tr("Dateiinhalt wurde während der Sicherung verändert oder konnte nicht geprüft werden: %1").arg(current_),true);return;}
    if(finalPass_) {++digestIndex_;nextDigest();}
    else {++filesDone_;nextFile();}
}
void BackupSession::nextDigest() {
    if(digestIndex_<files_.size())issue(Digest,files_[digestIndex_]);
    else startVerification();
}
void BackupSession::startVerification() {
    pending_=-1;current_.clear();state_="verifying";emit changed();
    const auto store=store_;const auto id=id_.toStdString();
    struct Result {bool success=false;QString error;};auto result=std::make_shared<Result>();
    verification_=QThread::create([store,id,result] {
        try {store->confirmInventory(id,"/int");store->confirmInventory(id,"/ext");store->finish(id);
            if(!store->verify(id))throw std::runtime_error("Final backup integrity check failed");result->success=true;
        }catch(const std::exception &e){result->error=QString::fromUtf8(e.what());try{store->interrupt(id,e.what());}catch(...) {}}
    });
    connect(verification_,&QThread::finished,this,[this,result] {
        auto *thread=verification_;verification_=nullptr;thread->deleteLater();
        error_=result->error;finalize(result->success,true);
    });verification_->start();
}
void BackupSession::failed(quint64 token,const QString &reason,bool confirmed) {
    if(!active_ || cancelled_ || token!=token_ || state_=="verifying")return;
    finishError(reason,confirmed);
}
void BackupSession::finishError(const QString &reason,bool confirmed) {error_=reason;finalize(false,confirmed);}
void BackupSession::finalize(bool success,bool confirmed) {
    if(!success && store_ && !id_.isEmpty()) {try{store_->interrupt(id_.toStdString(),error_.toStdString());}catch(...) {}}
    verified_=success;uncertain_=!success && !confirmed && online_ && !cancelled_;
    if(!job_.isEmpty()) {if(confirmed)jobs_.complete(job_.toStdString());else jobs_.fail(job_.toStdString(),error_.toStdString());}
    store_.reset();active_=false;pending_=-1;++token_;
    state_=success?"verified":cancelled_?"cancelled":uncertain_?"uncertain":"interrupted";
    emit changed();emit saved();
}
void BackupSession::cancel() {
    if(!canCancel())return;
    cancelled_=true;state_="stopping";error_=tr("Sicherung vom Nutzer abgebrochen.");emit changed();emit transportStopRequested();
}
QUrl BackupSession::backupUrl() const {
    static const QRegularExpression valid("^[0-9a-f]{32}$");
    return valid.match(id_).hasMatch()?QUrl::fromLocalFile(root_+"/backups/"+id_):QUrl();
}
