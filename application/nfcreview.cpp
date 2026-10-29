#include "nfcreview.h"
#include "applicationbackend.h"
#include "workbench/nfccontroller.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QQmlError>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTest>
#include <QTimer>
#include <stdexcept>
#include <algorithm>
#include <QCoreApplication>
#include <QImage>
namespace {
QQuickItem *findItem(QQuickItem *item, const QString &name) {
    if (item->objectName()==name) return item;
    for (auto *child:item->childItems()) if (auto *found=findItem(child,name)) return found;
    return nullptr;
}
class NfcReview : public QObject {
 public:
    NfcReview(QQmlApplicationEngine &engine, ApplicationBackend &backend, QString directory, bool card)
        : QObject(&engine), backend_(backend), directory_(std::move(directory)), card_(card) {
        window_=qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
        if (!window_ || !QDir().mkpath(directory_)) throw std::runtime_error("Cannot initialize NFC UI review");
        connect(&engine,&QQmlApplicationEngine::warnings,this,[this](const QList<QQmlError> &warnings) {
            for(const auto &w:warnings) errors_.append(w.toString());
        });
        timer_.setInterval(150); connect(&timer_,&QTimer::timeout,this,[this]{ tick(); });timer_.start();
        deadline_=QDateTime::currentMSecsSinceEpoch()+60000;
    }
 private:
    void require(bool condition,const char *why) { if(!condition) throw std::runtime_error(why); }
    void click(const QString &name) {
        auto *item=findItem(window_->contentItem(),name);
        require(item && item->isVisible() && item->isEnabled(),qPrintable("Unavailable UI control: "+name));
        for(auto *parent=item->parentItem();parent;parent=parent->parentItem()) {
            if(parent->property("contentY").isValid() && parent->property("contentHeight").isValid()) {
                const auto y=item->mapToItem(parent,QPointF(0,0)).y();
                if(y<0 || y+item->height()>parent->height()) {
                    const double max=std::max(0.0,parent->property("contentHeight").toDouble()-parent->height());
                    parent->setProperty("contentY",std::max(0.0,std::min(max,parent->property("contentY").toDouble()+y-parent->height()/2)));
                    QTest::qWait(100);
                }
            }
        }
        const auto p=item->mapToScene(QPointF(item->width()/2,item->height()/2));
        require(p.x()>=0 && p.y()>=0 && p.x()<window_->width() && p.y()<window_->height(),"Control outside native viewport");
        QTest::mouseClick(window_,Qt::LeftButton,Qt::NoModifier,p.toPoint(),30);++clicks_;
        QFile trace(directory_+"/clicks.jsonl");
        if(trace.open(QIODevice::Append)) trace.write(QJsonDocument(QJsonObject{{"control",name},{"x",p.x()},{"y",p.y()},
            {"nfcState",backend_.nfc()->state()},{"canStart",backend_.nfc()->canStart()},{"active",backend_.nfc()->active()}}).toJson(QJsonDocument::Compact)+"\n");
    }
    void screenshot(const QString &name) { require(window_->grabWindow().save(directory_+"/"+name+".png"),"Cannot save NFC screenshot"); }
    QString page() { auto *shell=findItem(window_->contentItem(),"workbench");return shell?shell->property("currentPage").toString():QString(); }
    void tick() {
        if(busy_)return;
        busy_=true;
        try {
            require(QDateTime::currentMSecsSinceEpoch()<deadline_,"NFC review timed out");
            auto *nfc=backend_.nfc();
            if(step_==0 && nfc->canStart()) { click("nav-nfc");++step_; }
            else if(step_==1 && findItem(window_->contentItem(),"nfc-start")) { screenshot("01_nfc_idle");click("nfc-start");++step_; }
            else if(step_==2 && card_ && !nfc->active()) {
                require(nfc->state()=="completed" && nfc->responses().size()==2 && nfc->stopConfirmed() && nfc->recordingVerified(),"Card read did not complete with verified recording and Stop");
                screenshot("02_nfc_read");recording_=nfc->recordingId();click("nfc-open-"+recording_);step_=7;
            } else if(step_==2 && !card_ && nfc->state()=="searching") { screenshot("02_nfc_searching");click("nav-screen");++step_; }
            else if(step_==3) { click("nfc-keep-reading");require(nfc->active() && page()=="nfc","Cancel interrupted measurement");click("nav-library");++step_; }
            else if(step_==4) { screenshot("03_nfc_conflict");click("nfc-confirm-stop");++step_; }
            else if(step_==5 && !nfc->active()) {
                require(nfc->stopConfirmed() && nfc->recordingVerified() && page()=="library","Confirmed switch did not wait for Stop");
                recording_=nfc->recordingId();click("nav-nfc");++step_;
            } else if(step_==6) { click("nfc-open-"+recording_);++step_; }
            else if(step_==7) {
                require(nfc->state()=="saved" && nfc->recordingVerified(),"Saved session did not reopen");
                require(errors_.isEmpty(),"QML warnings during NFC review");screenshot("04_nfc_saved");
                QJsonObject result{{"success",true},{"cardTest",card_},{"clicks",clicks_},{"recordingId",recording_},
                    {"responseCount",nfc->responses().size()},{"recordingVerified",true},{"qmlWarnings",errors_.size()},
                    {"finishedAt",QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}};
                QFile file(directory_+"/result.json");require(file.open(QIODevice::WriteOnly),"Cannot save result");file.write(QJsonDocument(result).toJson());
                timer_.stop();QCoreApplication::exit(0);
            }
        } catch(const std::exception &e) {
            timer_.stop();QFile f(directory_+"/failure.txt");if(f.open(QIODevice::WriteOnly))f.write(e.what());
            QFile state(directory_+"/failure-state.json");
            if(state.open(QIODevice::WriteOnly))state.write(QJsonDocument(QJsonObject{{"step",step_},{"clicks",clicks_},
                {"state",backend_.nfc()->state()},{"active",backend_.nfc()->active()},{"canStart",backend_.nfc()->canStart()},
                {"error",backend_.nfc()->error()},{"qmlWarnings",errors_.join("\n")}}).toJson());
            backend_.nfc()->stop();QTimer::singleShot(5500,this,[]{QCoreApplication::exit(1);});
        }
        busy_=false;
    }
    ApplicationBackend &backend_;QString directory_,recording_;QStringList errors_;QQuickWindow *window_=nullptr;
    QTimer timer_;qint64 deadline_;int step_=0,clicks_=0;bool card_=false,busy_=false;
};
}
void runNfcReview(QQmlApplicationEngine &engine,ApplicationBackend &backend,const QString &directory,bool card) {
    try {new NfcReview(engine,backend,directory,card);}
    catch(const std::exception &e) {qCritical("NFC review: %s",e.what());QCoreApplication::exit(1);}
}
