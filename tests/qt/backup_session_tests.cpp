#include "workbench/backupsession.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QEventLoop>
#include <QTimer>
#include <QTemporaryDir>
#include <QDebug>
#include <stdexcept>
#include <deque>
#include <functional>
namespace {
void require(bool ok,const char *why) {if(!ok)throw std::runtime_error(why);}
struct Request {int command;QString path;quint64 token;};
struct Harness {
    BackupSession session;
    QString root;
    std::deque<Request> requests;
    QMap<QString,QByteArray> content{{"/int/.settings","internal"},{"/ext/empty-file",{}},{"/ext/nested/data",QByteArray(200000,'a')}};
    std::function<bool(const Request &)> intercept;
    int inventoryPass=0,digestCount=0,stops=0;
    Harness(const QString &path):session(path),root(path) {
        QObject::connect(&session,&BackupSession::commandRequested,[this](int c,const QString &p,quint64 t){requests.push_back({c,p,t});});
        QObject::connect(&session,&BackupSession::transportStopRequested,[this]{++stops;});
        session.setConnection("fixture",true);
    }
    Request pop() {require(!requests.empty(),"Missing transport command");auto r=requests.front();requests.pop_front();return r;}
    void respond(const Request &r) {
        if(intercept && intercept(r))return;
        if(r.command==BackupSession::StorageInfo)session.storageInfo(r.token,true);
        else if(r.command==BackupSession::List) {
            if(r.path=="/int") {++inventoryPass;session.listed(r.token,{{"/int/.settings",8,false}});}
            else if(r.path=="/ext") session.listed(r.token,{{"/ext/empty-file",0,false},{"/ext/nested",0,true},{"/ext/empty-dir",0,true}});
            else if(r.path=="/ext/nested")session.listed(r.token,{{"/ext/nested/data",200000,false}});
            else session.listed(r.token,{});
        } else if(r.command==BackupSession::Read) {
            auto *sink=session.sink();require(sink->open(QIODevice::WriteOnly),"Cannot open backup sink");
            auto data=content[r.path];for(qsizetype i=0;i<data.size();i+=1024)require(sink->write(data.mid(i,1024))==qMin(qsizetype(1024),data.size()-i),"Sink failed");
            sink->close();session.readFinished(r.token);
        } else {++digestCount;session.digested(r.token,QCryptographicHash::hash(content[r.path],QCryptographicHash::Md5).toHex());}
    }
    void run() {
        int limit=1000;
        while(!requests.empty() && session.active()) {require(--limit>0,"Unbounded command loop");respond(pop());}
        if(session.state()=="verifying") {
            QEventLoop loop;
            auto c=QObject::connect(&session,&BackupSession::changed,[&]{if(!session.active())loop.quit();});
            QTimer::singleShot(5000,&loop,&QEventLoop::quit);loop.exec();QObject::disconnect(c);
        }
    }
};
void complete(const QString &path) {
    Harness h(path);require(h.session.begin({{"fixture",true}}),"Cannot begin backup");
    const auto first=h.requests.front();h.session.storageInfo(first.token-1,false);
    require(h.session.active(),"Stale response interrupted backup");h.run();
    require(h.session.verified() && !h.session.active() && h.inventoryPass==2 && h.digestCount==6,"Full backup did not check both inventories and content twice");
    require(h.session.filesDone()==3 && h.session.bytesDone()==200008,"Byte or file accounting lost empty files");
    UFlipper::BackupStore store(path.toStdString());const auto id=h.session.backupId().toStdString();
    require(store.verify(id) && store.get(id).scope==UFlipper::BackupScope::FullDevice,"Saved full backup failed independent verification");
    bool empty=false,hidden=false,emptyFile=false;
    for(const auto &e:store.entries(id)) {empty|=e.path=="/ext/empty-dir" && e.directory;hidden|=e.path=="/int/.settings";emptyFile|=e.path=="/ext/empty-file" && e.bytes==0 && !e.directory;}
    require(empty && hidden && emptyFile,"Lost hidden content, empty file or directory");
}
void rejection(const QString &path,int scenario) {
    Harness h(path);h.intercept=[&](const Request &r) {
        if(scenario==0 && r.command==BackupSession::StorageInfo && r.path=="/ext") {h.session.storageInfo(r.token,false);return true;}
        if(scenario==1 && r.command==BackupSession::List && r.path=="/ext") {h.session.listed(r.token,{{"/ext/../escape",1,false}});return true;}
        if(scenario==2 && h.inventoryPass==2 && r.command==BackupSession::List && r.path=="/ext") {h.session.listed(r.token,{});return true;}
        if(scenario==3 && h.digestCount==3 && r.command==BackupSession::Digest) {h.session.digested(r.token,QByteArray(32,'0'));return true;}
        if(scenario==4 && r.command==BackupSession::Read) {auto*s=h.session.sink();s->open(QIODevice::WriteOnly);s->write("excess-data");s->close();h.session.setConnection("fixture",false);return true;}
        if(scenario==5 && r.command==BackupSession::Read) {h.session.failed(r.token,"Transport timeout");return true;}
        return false;
    };
    require(h.session.begin({}),"Cannot start rejection fixture");h.run();
    require(!h.session.active() && !h.session.verified() && !h.session.error().isEmpty(),"Unsafe backup accepted");
    require(h.session.blocked()==(scenario==5),"Unconfirmed transport failure released device lease");
    UFlipper::BackupStore store(path.toStdString());require(store.get(h.session.backupId().toStdString()).state==UFlipper::BackupState::Interrupted,"Failure lost interrupted catalog entry");
}
void cancellation(const QString &path) {
    Harness h(path);require(h.session.begin({}),"Cannot begin cancellation");auto r=h.pop();
    h.session.cancel();require(h.stops==1 && h.session.active() && !h.session.canCancel(),"Cancel released device before transport closed");
    h.session.storageInfo(r.token,true);require(h.requests.empty(),"Late response after cancellation started new work");
    h.session.failed(r.token,"Aborted RPC");require(h.session.active(),"Operation callback bypassed transport stop");
    h.session.setConnection("fixture",false);require(!h.session.active() && h.session.state()=="cancelled","Closed transport did not finish cancellation");
    h.session.setConnection("fixture",true);require(h.session.begin({}),"Fresh handshake did not permit next backup");h.session.setConnection("fixture",false);
}
void unavailable(const QString &path) {
    QFile file(path);require(file.open(QIODevice::WriteOnly),"Cannot create non-directory fixture");file.close();
    Harness h(path);require(!h.session.begin({}) && h.requests.empty() && !h.session.error().isEmpty(),"Device accessed before local storage was writable");
}
}
int main(int argc,char**argv) {
    QCoreApplication app(argc,argv);QTemporaryDir root;
    try {require(root.isValid(),"Cannot create test directory");complete(root.path()+"/complete");for(int i=0;i<6;++i)rejection(root.path()+"/reject"+QString::number(i),i);
        cancellation(root.path()+"/cancel");unavailable(root.path()+"/file");qInfo()<<"PASS: full inventory, streamed content, empty/hidden entries, changed inventory/digest, missing SD, unsafe path, unwritable sink, uncertain transport, cancellation and reconnect";return 0;
    }catch(const std::exception &e){qCritical()<<e.what();return 1;}
}
