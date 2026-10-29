#include "backupcontroller.h"
#include "deviceregistry.h"
#include "flipperzero/devicestate.h"
#include "flipperzero/flipperzero.h"
#include "flipperzero/protobufsession.h"
#include "flipperzero/rpc/storageinfooperation.h"
#include "flipperzero/rpc/storagelistoperation.h"
#include "flipperzero/rpc/storagereadoperation.h"
#include "flipperzero/rpc/storagemd5sumoperation.h"
#include "flipperzero/screenstreamer.h"
#include <QDateTime>
#include <QStandardPaths>
BackupController::BackupController(Flipper::DeviceRegistry *registry, Flipper::Zero::ScreenStreamer *screen,
                                 std::function<bool()> idle, QObject *parent)
    : BackupSession(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+"/uFlipper/workbench",parent),
      registry_(registry),screen_(screen),idle_(std::move(idle)) {
    connect(registry_,&Flipper::DeviceRegistry::currentDeviceChanged,this,&BackupController::attach);
    connect(this,&BackupSession::commandRequested,this,&BackupController::dispatch);
    connect(this,&BackupSession::transportStopRequested,this,[this] {
        // Storage RPC has no per-read cancellation: closing this owner's transport
        // prevents late chunks being delivered into a later backup or device action.
        if(device_ && device_->rpc()->isSessionUp()) device_->rpc()->stopSession();
        else connectionChanged();
    });
    attach();
}
QString BackupController::limitation() const {
    return tr("Vollbackup noch gesperrt: Die Geräte-Dateiliste filtert Dateinamen. Für eine nachweislich vollständige SD-Sicherung fehlt ein ungefiltertes Inventar. Eine zusätzliche Anbindung ist erforderlich; neue SD-Helfer werden erst nach ausdrücklicher Freigabe installiert.");
}
bool BackupController::canStart() const {
    return limitation().isEmpty() && available() && idle_() && device_ && device_->deviceState()->isOnline() &&
           !device_->deviceState()->isRecoveryMode() && device_->rpc()->isSessionUp();
}
void BackupController::attach() {
    for(const auto &c:connections_)QObject::disconnect(c);
    connections_.clear();device_=registry_->currentDevice();
    if(device_) {
        connections_<<connect(device_->deviceState(),&Flipper::Zero::DeviceState::isOnlineChanged,this,&BackupController::connectionChanged);
        connections_<<connect(device_->rpc(),&Flipper::Zero::ProtobufSession::sessionStateChanged,this,&BackupController::connectionChanged);
    }
    connectionChanged();
}
void BackupController::connectionChanged() {
    setConnection(device_?device_->deviceState()->deviceInfo().systemLocation:QString(),
                  device_ && device_->deviceState()->isOnline() && device_->rpc()->isSessionUp());
}
void BackupController::start() {
    if(!canStart())return;
    const auto &info=device_->deviceState()->deviceInfo();
    begin({{"deviceName",info.name},{"firmwareVersion",info.firmware.version},{"firmwareCommit",info.firmware.commit},
           {"hardwareTarget",info.hardware.target},{"rpcMajor",info.protobuf.versionMajor},{"rpcMinor",info.protobuf.versionMinor},
           {"startedAt",QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
           {"contentVerification","remote-md5-before-and-after-inventory;local-sha256"},
           {"storageScope","RPC /int and /ext; firmware aliases retained; no raw flash image"}});
}
void BackupController::dispatch(int command,const QString &path,quint64 token) {
    if(!device_ || !device_->rpc()->isSessionUp()) {failed(token,tr("Die Geräteverbindung ist nicht verfügbar."));return;}
    auto *rpc=device_->rpc();
    if(command==StorageInfo) {
        // Local storage and the catalog are ready before the first device action.
        // Stop the stream through this same owner before listing storage.
        if(path=="/int")screen_->stop();
        auto *op=rpc->storageInfo(path.toUtf8());watch(op,token,[this,op,token]{storageInfo(token,op->isPresent());});
    } else if(command==List) {
        auto *op=rpc->storageList(path.toUtf8());watch(op,token,[this,op,token] {
            QList<BackupRemoteEntry> entries;
            for(const auto &file:op->files()) {
                const auto path=QString::fromUtf8(file.absolutePath);
                if(file.size<0 || path.toUtf8()!=file.absolutePath || (file.type!=FileType::Directory && file.type!=FileType::RegularFile)) {
                    failed(token,tr("Nicht unterstützter Geräteeintrag im Backupinventar."),true);return;
                }
                entries.append({path,quint64(file.size),file.type==FileType::Directory});
            }
            listed(token,entries);
        });
    } else if(command==Read) {
        auto *op=rpc->storageRead(path.toUtf8(),sink());watch(op,token,[this,token]{readFinished(token);});
    } else if(command==Digest) {
        auto *op=rpc->storageMd5Sum(path.toUtf8());watch(op,token,[this,op,token]{digested(token,op->md5Sum());});
    }
}
void BackupController::watch(AbstractOperation *operation,quint64 token,std::function<void()> success) {
    operation->setTimeout(15000);const auto expected=device_;
    auto finished=std::make_shared<bool>(false);
    connect(operation,&AbstractOperation::finished,this,[this,operation,token,expected,success=std::move(success),finished] {
        *finished=true;
        if(device_!=expected || !active())return;
        if(operation->isError()) {
            failed(token,operation->errorString());
            if(expected && expected->rpc()->isSessionUp())expected->rpc()->stopSession();
        } else success();
    });
    connect(operation,&QObject::destroyed,this,[this,token,expected,finished] {
        if(!*finished && device_==expected) {
            failed(token,tr("Die Geräteaktion wurde vor ihrer Bestätigung abgebrochen."));
            if(expected && expected->rpc()->isSessionUp())expected->rpc()->stopSession();
        }
    });
}
