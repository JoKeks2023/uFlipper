#include "viccbridge.h"
#include "nfcsession.h"
#include <QHostAddress>
ViccBridge::ViccBridge(NfcSession &session, QObject *parent) : QObject(parent), session_(session) {
    server_.setMaxPendingConnections(1);
    connect(&server_, &QTcpServer::newConnection, this, &ViccBridge::acceptClient);
    connect(&session_, &NfcSession::changed, this, [this] {
        if (session_.active() && session_.mode() == "reader") sessionSeen_ = true;
        if (sessionSeen_ && !session_.active()) stop();
        else process();
    });
    connect(&session_, &NfcSession::readerResponse, this, [this](const QByteArray &response) {
        if (!waitingResponse_) return;
        waitingResponse_ = false; send(response); deadline_.stop(); process();
    });
    activationWait_.setSingleShot(true);
    connect(&activationWait_, &QTimer::timeout, this, [this] { powerPending_ = false; process(); });
    deadline_.setSingleShot(true);
    connect(&deadline_, &QTimer::timeout, this, [this] { fail(tr("Zeitüberschreitung der lokalen Reader-Verbindung.")); });
}
bool ViccBridge::start(quint16 port) {
    if (server_.isListening()) return false;
    error_.clear(); sessionSeen_ = false;
    if (!server_.listen(QHostAddress::LocalHost, port)) {
        error_ = tr("Reader-Port nicht verfügbar: %1").arg(server_.errorString()); emit changed(); return false;
    }
    emit changed(); return true;
}
void ViccBridge::stop() {
    sessionSeen_ = false; waitingResponse_ = false; powerPending_ = false; deadline_.stop();activationWait_.stop();
    server_.close(); queue_.clear(); input_.clear();
    if (client_) { auto *socket=client_.data();client_=nullptr;socket->disconnect(this);socket->abort();socket->deleteLater(); }
    emit changed();
}
QString ViccBridge::status() const {
    if (!error_.isEmpty()) return error_;
    if (!server_.isListening()) return tr("Ausgeschaltet");
    if (!client_) return tr("Warte auf macOS-Reader-Komponente");
    return session_.readerReady() ? tr("Verbunden · Karte bereit") : tr("Verbunden · keine Karte bereit");
}
void ViccBridge::acceptClient() {
    while (server_.hasPendingConnections()) {
        auto *socket=server_.nextPendingConnection();
        if (client_ || socket->peerAddress()!=QHostAddress::LocalHost) { socket->abort();socket->deleteLater();continue; }
        client_=socket;socket->setReadBufferSize(4096);
        connect(socket,&QTcpSocket::readyRead,this,&ViccBridge::receive);
        connect(socket,&QTcpSocket::disconnected,this,[this,socket] {
            if(client_!=socket)return;
            client_=nullptr; input_.clear();queue_.clear();
            if(waitingResponse_) { waitingResponse_=false;session_.stop(); }
            deadline_.stop();socket->deleteLater();emit changed();
        });
        emit changed();
    }
}
void ViccBridge::receive() {
    if(!client_)return;
    input_+=client_->readAll();
    while(input_.size()>=2) {
        const unsigned length=(unsigned(uint8_t(input_[0]))<<8)|uint8_t(input_[1]);
        if(!length || length>240 || queue_.size()>=4) { fail(tr("Ungültiger oder zu großer Reader-Auftrag."));return; }
        if(input_.size()<int(length)+2)break;
        queue_.enqueue(input_.mid(2,length));input_.remove(0,int(length)+2);
    }
    if(input_.size()>242) {fail(tr("Reader-Eingabepuffer überschritten."));return;}
    process();
}
void ViccBridge::send(const QByteArray &payload) {
    if(!client_)return;
    QByteArray frame;frame.append(char(payload.size()>>8));frame.append(char(payload.size()));frame+=payload;
    if(client_->write(frame)!=frame.size())fail(tr("Reader-Antwort konnte nicht übertragen werden."));
}
void ViccBridge::process() {
    if(processing_ || waitingResponse_ || !client_ || queue_.isEmpty())return;
    processing_=true;
    while(client_ && !queue_.isEmpty() && !waitingResponse_) {
        const auto request=queue_.head();
        if(request.size()==1) {
            const auto control=uint8_t(request[0]);
            if(control==4) {
                if(session_.state()=="powered-off" && session_.readerPower(true)) {powerPending_=true;activationWait_.start(3500);}
                if(powerPending_ && !session_.readerReady())break;
                powerPending_=false;activationWait_.stop();queue_.dequeue();
                // PC/SC transport ATR for a T=1 APDU proxy, not an ATR read from
                // the contactless card. Actual card responses are never synthesized.
                send(session_.readerReady() ? QByteArray::fromHex("3b800181") : QByteArray());
                continue;
            }
            if(control>2) {fail(tr("Unbekanntes Reader-Steuerkommando."));break;}
            if(!session_.readerPower(control!=0,control==2)) {
                if(session_.active() && session_.mode()=="reader") {if(!deadline_.isActive())deadline_.start(8000);break;}
                fail(tr("NFC-Sitzung für Reader-Auftrag nicht bereit."));break;
            }
            if(control!=0 && !session_.readerReady()) {powerPending_=true;activationWait_.start(3500);}
            queue_.dequeue();deadline_.stop();
        } else {
            if(request.size()<4 || (request.size()>5 && request[4]==0)) {fail(tr("Dieser Helfer unterstützt nur kurze APDU-Kommandos."));break;}
            if(!session_.readerReady()) {fail(tr("Keine aktive Karte für den Reader-Auftrag."));break;}
            queue_.dequeue();waitingResponse_=true;deadline_.start(6000);
            if(!session_.readerTransmit(request)) {waitingResponse_=false;fail(tr("Reader-Kommando überschreitet die Karten- oder Helfergrenze."));break;}
        }
    }
    processing_=false;
}
void ViccBridge::fail(const QString &reason) {
    error_=reason;stop();
    if(session_.active() && session_.mode()=="reader")session_.stop();
}
