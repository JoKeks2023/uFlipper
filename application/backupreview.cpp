#include "backupreview.h"
#include "applicationbackend.h"
#include "workbench/backupcontroller.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QQmlError>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTest>
#include <QTimer>
#include <stdexcept>
namespace {
QQuickItem *find(QQuickItem *item,const QString &name) {
    if(item->objectName()==name)return item;
    for(auto *c:item->childItems())if(auto*f=find(c,name))return f;
    return nullptr;
}
class Review:public QObject {
 public:
    Review(QQmlApplicationEngine &engine,ApplicationBackend &backend,QString directory,bool cancel)
        :QObject(&engine),backend_(backend),directory_(std::move(directory)),cancel_(cancel) {
        window_=qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
        if(!window_ || !QDir().mkpath(directory_))throw std::runtime_error("Cannot initialize native backup review");
        connect(&engine,&QQmlApplicationEngine::warnings,this,[this](const QList<QQmlError>&warnings){for(const auto&w:warnings)warnings_.append(w.toString());});
        timer_.setInterval(200);connect(&timer_,&QTimer::timeout,this,[this]{tick();});timer_.start();
        deadline_=QDateTime::currentMSecsSinceEpoch()+7200000;
    }
 private:
    void require(bool ok,const char*why){if(!ok)throw std::runtime_error(why);}
    void click(const QString &name) {
        auto *item=find(window_->contentItem(),name);
        require(item && item->isVisible() && item->isEnabled(),qPrintable("Unavailable backup control: "+name));
        auto p=item->mapToScene(QPointF(item->width()/2,item->height()/2));
        require(p.y()>=0 && p.y()<window_->height(),"Backup control outside viewport");
        QTest::mouseClick(window_,Qt::LeftButton,Qt::NoModifier,p.toPoint(),30);++clicks_;
    }
    void screenshot(const QString &name){require(window_->grabWindow().save(directory_+"/"+name+".png"),"Cannot save backup screenshot");}
    void tick() {
        if(busy_)return;busy_=true;
        try {
            require(QDateTime::currentMSecsSinceEpoch()<deadline_,"Backup review timed out");auto*b=backend_.backup();
            if(step_==0 && b->canStart()){click("nav-backups");++step_;}
            else if(step_==1 && find(window_->contentItem(),"backup-start")){screenshot("01_backup_idle");click("backup-start");++step_;}
            else if(step_==2 && b->active()) {id_=b->backupId();screenshot("02_backup_running");click("nav-library");++step_;}
            else if(step_==3) {screenshot("03_backup_conflict");click("backup-keep-copying");require(b->active(),"Keep copying stopped backup");++step_;}
            else if(step_==4 && cancel_) {click("backup-cancel");++step_;}
            else if(step_==4 && !cancel_ && !b->active()) {require(b->verified(),qPrintable("Full backup failed: "+b->error()));step_=5;}
            else if(step_==5 && !b->active()) {
                require(cancel_?b->state()=="cancelled":b->verified(),"Incorrect backup final state");
                require(warnings_.isEmpty(),qPrintable(warnings_.join("\n")));screenshot("04_backup_result");
                const QJsonObject result{{"success",true},{"cancelTest",cancel_},{"clicks",clicks_},{"backupId",id_},
                    {"state",b->state()},{"verified",b->verified()},{"bytesDone",double(b->bytesDone())},{"bytesTotal",double(b->bytesTotal())},
                    {"filesDone",b->filesDone()},{"filesTotal",b->filesTotal()},{"qmlWarnings",warnings_.size()},
                    {"finishedAt",QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}};
                QFile f(directory_+"/result.json");require(f.open(QIODevice::WriteOnly),"Cannot save backup result");f.write(QJsonDocument(result).toJson());timer_.stop();QCoreApplication::exit(0);
            } else if(step_>=2 && !b->active() && !b->verified() && b->state()!="cancelled") throw std::runtime_error(("Backup stopped: "+b->error()).toStdString());
        }catch(const std::exception&e) {
            timer_.stop();QFile f(directory_+"/failure.json");auto*b=backend_.backup();
            if(f.open(QIODevice::WriteOnly))f.write(QJsonDocument(QJsonObject{{"failure",e.what()},{"step",step_},{"state",b->state()},
                {"error",b->error()},{"backupId",b->backupId()},{"filesDone",b->filesDone()},{"bytesDone",double(b->bytesDone())},
                {"filesTotal",b->filesTotal()},{"bytesTotal",double(b->bytesTotal())},{"qmlWarnings",warnings_.join("\n")}}).toJson());
            b->cancel();QTimer::singleShot(3000,this,[]{QCoreApplication::exit(1);});
        }
        busy_=false;
    }
    ApplicationBackend &backend_;QString directory_,id_;bool cancel_,busy_=false;QQuickWindow *window_;QTimer timer_;
    QStringList warnings_;qint64 deadline_;int step_=0,clicks_=0;
};
}
void runBackupReview(QQmlApplicationEngine&engine,ApplicationBackend&backend,const QString&directory,bool cancel) {
    try{new Review(engine,backend,directory,cancel);}catch(const std::exception&e){qCritical("Backup review: %s",e.what());QCoreApplication::exit(1);}
}
