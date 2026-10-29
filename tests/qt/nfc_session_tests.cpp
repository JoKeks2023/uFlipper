#include "workbench/nfcsession.h"
#include <QCoreApplication>
#include <QFile>
#include <QTemporaryDir>
#include <QVariantMap>
#include <stdexcept>
#include <vector>
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
    h.session.openRecording(id);
    require(h.session.state()=="saved" && h.session.responses().size()==2 && h.session.card().value("uid").toString()=="01 02 03 04", "Saved session did not reopen");
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
        completedAndReopened(tmp.path()+"/complete");
        stopAndStaleAck(tmp.path()+"/stop");
        disconnectAndFailedTransport(tmp.path()+"/disconnect");
        unavailableRecording(tmp.path()+"/file");
        return 0;
    } catch(const std::exception &e) { qCritical("NFC session test: %s",e.what());return 1; }
}
