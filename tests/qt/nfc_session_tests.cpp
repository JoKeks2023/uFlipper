#include "workbench/nfcsession.h"
#include "workbench/viccbridge.h"
#include <QTcpSocket>
#include <QHostAddress>
#include <QElapsedTimer>
#include <QThread>
#include <QCoreApplication>
#include <QFile>
#include <QEventLoop>
#include <QTimer>
#include <QTemporaryDir>
#include <QVariantMap>
#include <stdexcept>
#include <vector>
#include <functional>
namespace {
void require(bool value, const char *reason) { if (!value) throw std::runtime_error(reason); }
struct Sent { int command; QByteArray frame; quint64 token; };
struct Harness {
    NfcSession session;
    std::vector<Sent> sent;
    explicit Harness(const QString &root) : session(root) {
        QObject::connect(&session, &NfcSession::commandRequested, [&] (int c, const QByteArray &f, quint64 t) { sent.push_back({c,f,t}); });
        session.setConnection("test-device", true);
    }
    Sent current() { require(!sent.empty(), "No transport request"); return sent.back(); }
    void ack() { session.acknowledge(current().token); }
    void reply(QByteArray payload, int status = 0) {
        const auto request = current();
        QByteArray frame(8, '\0');
        frame[0]='U'; frame[1]='F'; frame[2]=1; frame[3]=char(request.command|0x80);
        frame[4]=request.frame[4]; frame[5]=char(status); frame[6]=char(payload.size()); frame[7]=char(payload.size()>>8);
        frame+=payload; session.receive(frame);
    }
    void info() {
        session.helperState(true); ack();
        require(current().command==NfcSession::Info,"Started before AppStarted and start ACK");
        reply("uf-nfc/0.1.0;iso14443-4a;short-apdu;max=240");
        require(current().command==NfcSession::Info,"Helper reply bypassed delivery ACK");
        ack();
        require(current().command==NfcSession::Activate,"No activation after verified Info");
    }
    void card() {
        ack(); reply(QByteArray::fromHex("0401020304f000"));
        require(current().command==NfcSession::Transceive,"No PPSE request after activation");
        require(current().frame.mid(8)==QByteArray::fromHex("00a404000e325041592e5359532e444446303100"),"Unexpected APDU sent");
    }
    void stopped() {
        require(current().command==NfcSession::ExitHelper,"Stop did not request AppExit");
        session.helperState(false);
        require(session.active(),"AppClosed bypassed missing exit ACK");
        ack(); require(!session.active() && session.stopConfirmed(),"Stop not confirmed");
    }
};
void unchangedConnectionIsNotAnEvent(const QString &root) {
    Harness h(root);
    int changes=0;
    QObject::connect(&h.session,&NfcSession::changed,[&]{++changes;});
    for(int i=0;i<100;++i)h.session.setConnection("test-device",true);
    require(changes==0 && h.sent.empty(),"Idle/Running transport transitions notified a new device connection");
}
void completedAndReopened(const QString &root) {
    Harness h(root);
    require(h.session.begin({{"firmware","fixture"}}),"Cannot start session");
    require(QFile::exists(h.session.recordingUrl().toLocalFile()+"/events.bin"),"App started before recording file exists");
    h.info(); h.card();
    h.ack(); h.reply(QByteArray::fromHex("6f009000"));
    h.ack(); h.reply(QByteArray::fromHex("6f009000"));
    require(h.current().command==NfcSession::Deactivate,"No NFC deactivation");
    h.ack();h.reply({});h.stopped();
    require(h.session.state()=="completed" && h.session.recordingVerified(),"Completed recording not verified");
    const auto id=h.session.recordingId();
    const auto originalTimes=h.session.responses();
    h.session.openRecording(id);
    require(h.session.state()=="saved" && h.session.responses().size()==2 && h.session.card().value("uid").toString()=="01 02 03 04", "Saved session did not reopen");
    require(h.session.responses()==originalTimes,"Reopening changed original response times or data");
    QFile corrupt(h.session.recordingUrl().toLocalFile()+"/events.bin");
    require(corrupt.open(QIODevice::Append),"Cannot corrupt test fixture"); corrupt.write("bad");corrupt.close();
    h.session.openRecording(id);
    require(!h.session.recordingVerified() && h.session.responses().isEmpty() && !h.session.error().isEmpty(),"Corrupted capture was displayed as verified");
}
void stopAndStaleAck(const QString &root) {
    Harness h(root);h.session.begin({});h.info();
    const auto old=h.current();h.session.stop();
    h.session.acknowledge(old.token);
    require(h.session.active(),"Old ACK completed new Stop");
    h.stopped();
    require(h.session.state()=="stopped" && h.session.responses().isEmpty(),"User stop became completed card read");
    require(h.session.begin({}),"Confirmed Stop did not release resource lease");
    h.session.rejectStart("Missing approved helper");
    require(!h.session.active() && !h.session.blocked() && h.session.state()=="interrupted", "Preflight rejection started or retained hardware");
    require(h.session.begin({}),"Rejected preflight retained resource lease");
    h.session.rejectStart("test cleanup");
}
void cancelDuringPreflight(const QString &root) {
    Harness h(root);h.session.begin({});
    require(h.current().command==NfcSession::StartHelper,"No preflight request");
    h.session.stop();h.session.cancelBeforeStart();
    require(!h.session.active() && h.session.stopConfirmed() && h.session.recordingVerified(),"Preflight cancellation did not finish without starting hardware");
}
void disconnectAndFailedTransport(const QString &root) {
    Harness h(root);h.session.begin({});h.info();
    h.session.setConnection("test-device",false);
    require(!h.session.active() && !h.session.stopConfirmed() && h.session.recordingVerified(),"Disconnect lost capture or claimed confirmed Stop");
    h.session.setConnection("test-device",true);
    require(h.session.begin({}),"Reconnect did not allow a new session");
    h.info();h.card();h.ack();h.reply(QByteArray::fromHex("6a82"));
    require(!h.session.error().isEmpty(),"Unsupported PPSE response was successful");h.stopped();
    require(h.session.state()=="interrupted", "Failed read became completed");
    require(h.session.begin({}),"Stopped failed read retained hardware");h.info();h.session.stop();
    h.session.acknowledge(h.current().token,"Exit failed");
    require(!h.session.active() && h.session.blocked() && !h.session.available(),"Unconfirmed Stop released device");
    h.session.setConnection("test-device",false);h.session.setConnection("test-device",true);
    require(h.session.available(),"Reconnect did not clear uncertain device generation");
}
void responseAndStopTimeout(const QString &root) {
    Harness h(root);h.session.begin({});h.info();
    QEventLoop response;
    auto connection=QObject::connect(&h.session,&NfcSession::commandRequested,[&](int command,const QByteArray &,quint64) {
        if(command==NfcSession::ExitHelper) response.quit();
    });
    QTimer::singleShot(6500,&response,&QEventLoop::quit);
    response.exec();QObject::disconnect(connection);
    require(h.current().command==NfcSession::ExitHelper && !h.session.error().isEmpty(),"Missing card response did not request Stop");
    QEventLoop stopping;
    connection=QObject::connect(&h.session,&NfcSession::changed,[&] {if(!h.session.active())stopping.quit();});
    QTimer::singleShot(6500,&stopping,&QEventLoop::quit);
    stopping.exec();QObject::disconnect(connection);
    require(!h.session.active() && h.session.blocked() && !h.session.stopConfirmed() && h.session.recordingVerified(),
            "Stop timeout released hardware or lost interrupted recording");
}
void ndefReadAndReplay(const QString &root) {
    Harness h(root);require(h.session.begin({},"ndef"),"NDEF start failed");h.info();
    h.ack();h.reply(QByteArray::fromHex("0401020304f000"));
    QByteArray selected,cc=QByteArray::fromHex("000f20008000ff0406e1047fff00ff"),message=QByteArray::fromHex("d101055402656e4869");
    QByteArray file=QByteArray::fromHex("0009")+message;bool application=false;
    int commands=0;
    while(h.current().command==NfcSession::Transceive) {
        require(++commands<20,"Unbounded NDEF read");
        const auto apdu=h.current().frame.mid(8);QByteArray response;
        if(apdu==QByteArray::fromHex("00a4040007d276000085010100"))application=true;
        else {
            require(application,"NDEF file access before application selection");
            if(apdu.startsWith(QByteArray::fromHex("00a4000c02")))selected=apdu.right(2);
            else {
                require(apdu.size()==5 && apdu[1]==char(0xb0),"NDEF used a write or unknown command");
                require(selected==QByteArray::fromHex("e103") || selected==QByteArray::fromHex("e104"),"NDEF read without selected file");
                const auto &data=selected==QByteArray::fromHex("e103")?cc:file;
                const int offset=(uint8_t(apdu[2])<<8)|uint8_t(apdu[3]),count=uint8_t(apdu[4]);
                require(offset+count<=data.size(),"NDEF read outside selected file");response=data.mid(offset,count);
            }
        }
        h.ack();h.reply(response+QByteArray::fromHex("9000"));
    }
    require(h.current().command==NfcSession::Deactivate && h.session.ndefRecords().size()==1,"NDEF did not finish with parsed records");
    require(h.session.ndefRecords()[0].toMap().value("content")=="Hi","NDEF text not decoded");
    h.ack();h.reply({});h.stopped();require(h.session.state()=="completed","NDEF completion not reported");
    const auto original=h.session.responses();const auto id=h.session.recordingId();h.session.openRecording(id);
    require(h.session.recordingVerified() && h.session.mode()=="ndef" && h.session.responses()==original && h.session.ndefRecords()[0].toMap().value("content")=="Hi","NDEF replay lost records or original times");
}
void failedNdefStillReopens(const QString &root) {
    Harness h(root);h.session.begin({},"ndef");h.info();h.ack();h.reply(QByteArray::fromHex("0401020304f000"));
    h.ack();h.reply(QByteArray::fromHex("6a82"));h.stopped();
    const auto id=h.session.recordingId();h.session.openRecording(id);
    require(h.session.state()=="saved" && h.session.recordingVerified() && h.session.responses().size()==1 && !h.session.error().isEmpty(),"Failed NDEF card read lost its intact original response on reopen");
}
void readerSession(const QString &root) {
    Harness h(root);require(h.session.begin({},"reader"),"Reader start failed");h.info();h.ack();h.reply(QByteArray::fromHex("0401020304f000"));
    require(h.session.readerReady() && h.current().command==NfcSession::Activate,"Reader sent an unsolicited APDU");
    QByteArray delivered;QObject::connect(&h.session,&NfcSession::readerResponse,[&](const QByteArray &b){delivered=b;});
    require(h.session.readerTransmit(QByteArray::fromHex("00a4040000")),"Reader APDU not accepted");
    require(!h.session.readerTransmit(QByteArray::fromHex("00b0000002")),"Concurrent reader APDU accepted");
    h.reply(QByteArray::fromHex("6a82"));require(delivered.isEmpty(),"Reader reply bypassed transport ACK");h.ack();
    require(delivered==QByteArray::fromHex("6a82") && h.session.readerReady(),"Card status was fabricated or treated as transport failure");
    require(h.session.readerPower(false),"Reader power off rejected");h.ack();h.reply({});
    require(h.session.state()=="powered-off" && h.session.card().isEmpty(),"Power off retained card state");
    require(h.session.readerPower(true),"Reader power on rejected");h.ack();h.reply(QByteArray::fromHex("0401020304f000"));
    require(h.session.readerPower(true,true),"Reader reset rejected");h.ack();h.reply({});
    require(h.current().command==NfcSession::Activate,"Reset did not reactivate card");h.ack();h.reply(QByteArray::fromHex("0401020304f000"));
    h.session.stop();h.stopped();require(h.session.recordingVerified(),"Reader recording lost");
    h.session.openRecording(h.session.recordingId());require(h.session.mode()=="reader" && h.session.responses()[0].toMap().value("statusWord")=="6A82","Reader replay did not preserve card status");
}
void spinUntil(const std::function<bool()> &done) {
    QElapsedTimer elapsed;elapsed.start();
    while(!done() && elapsed.elapsed()<2000) {QCoreApplication::processEvents();QThread::msleep(1);}
    require(done(),"Loopback reader contract timed out");
}
QByteArray readFrame(QTcpSocket &socket) {
    spinUntil([&]{return socket.bytesAvailable()>=2;});const auto size=socket.read(2);
    const auto n=(uint8_t(size[0])<<8)|uint8_t(size[1]);spinUntil([&]{return socket.bytesAvailable()>=n;});return socket.read(n);
}
QByteArray framed(const QByteArray &bytes) {QByteArray result;result.append(char(bytes.size()>>8));result.append(char(bytes.size()));return result+bytes;}
void readerLoopback(const QString &root) {
    Harness h(root);h.session.begin({},"reader");h.info();h.ack();h.reply(QByteArray::fromHex("0401020304f000"));
    ViccBridge bridge(h.session);require(bridge.start(0),"Cannot bind local reader contract endpoint");
    QTcpSocket socket;socket.connectToHost(QHostAddress::LocalHost,bridge.port());spinUntil([&]{return socket.state()==QAbstractSocket::ConnectedState;});
    // Driver contract peer is a test fixture, not an independent PC/SC proof.
    socket.write(QByteArray::fromHex("00"));socket.flush();QCoreApplication::processEvents();
    socket.write(QByteArray::fromHex("0104"));
    require(readFrame(socket)==QByteArray::fromHex("3b800181"),"Fragmented VICC ATR request failed");
    const auto apdu=QByteArray::fromHex("00a4040000");socket.write(framed(apdu));
    spinUntil([&]{return h.current().command==NfcSession::Transceive;});require(h.current().frame.mid(8)==apdu,"Bridge changed external APDU");
    h.ack();h.reply(QByteArray::fromHex("6a82"));require(readFrame(socket)==QByteArray::fromHex("6a82"),"Bridge did not pass original response");
    socket.write(QByteArray::fromHex("0101"));
    spinUntil([&]{return h.current().command==NfcSession::ExitHelper;});
    require(!bridge.error().isEmpty(),"Oversized reader frame was accepted");h.stopped();
}
void unavailableRecording(const QString &root) {
    QFile file(root);require(file.open(QIODevice::WriteOnly),"Cannot create fixture file");file.write("file");file.close();
    Harness h(root);
    require(!h.session.begin({}) && h.sent.empty(),"Device started without writable capture storage");
}
}
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);
    try {
        QTemporaryDir tmp;require(tmp.isValid(),"No fixture folder");
        ndefReadAndReplay(tmp.path()+"/ndef");
        failedNdefStillReopens(tmp.path()+"/ndef-failed");
        readerSession(tmp.path()+"/reader");
        readerLoopback(tmp.path()+"/loopback");
        unchangedConnectionIsNotAnEvent(tmp.path()+"/connection");
        completedAndReopened(tmp.path()+"/complete");
        stopAndStaleAck(tmp.path()+"/stop");
        cancelDuringPreflight(tmp.path()+"/preflight");
        disconnectAndFailedTransport(tmp.path()+"/disconnect");
        unavailableRecording(tmp.path()+"/file");
        responseAndStopTimeout(tmp.path()+"/timeout");
        return 0;
    } catch(const std::exception &e) { qCritical("NFC session test: %s",e.what());return 1; }
}
