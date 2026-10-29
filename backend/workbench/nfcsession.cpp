#include "nfcsession.h"
#include "../../helpers/nfc_bridge/bridge_protocol.h"
#include <QDateTime>
#include <QFile>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QtEndian>
#include <chrono>
#include <stdexcept>

NfcSession::NfcSession(const QString &root, QObject *parent) : QObject(parent), root_(root) {
    deadline_.setSingleShot(true);
    retry_.setSingleShot(true);
    retry_.setInterval(200);
    connect(&retry_, &QTimer::timeout, this, [this] { if (active_ && state_ == "searching") issue(Activate); });
    connect(&deadline_, &QTimer::timeout, this, [this] {
        if (pending_ == ExitHelper) {
            if (error_.isEmpty()) error_ = tr("Stop nicht bestätigt. Flipper neu verbinden.");
            finalize(false);
        } else abort(tr("Zeitüberschreitung bei der NFC-Kommunikation."));
    });
}
NfcSession::~NfcSession() {
    if (active_) {
        error_ = tr("App beendet, bevor der Geräte-Stop bestätigt wurde.");
        finalize(false);
    }
}
void NfcSession::setConnection(const QString &identity, bool online) {
    const bool replacement = online && (identity != identity_ || !online_);
    if (active_ && (!online || replacement)) {
        error_ = tr("Geräteverbindung unterbrochen. Die Aufnahme wurde abgeschlossen.");
        jobs_.disconnectDevice(error_.toStdString());
        finalize(false);
    }
    if (replacement) {
        identity_ = identity;
        jobs_.connectDevice(identity.toStdString());
        uncertain_ = false;
    }
    online_ = online;
    emit changed();
}
bool NfcSession::begin(const QVariantMap &metadata) {
    if (!available()) return false;
    recordingId_.clear(); job_.clear(); error_.clear(); card_.clear(); responses_.clear();
    verified_ = false; stopConfirmed_ = false; started_ = false; closed_ = false;
    try {
        store_ = std::make_unique<UFlipper::CaptureStore>(root_.toStdString());
        auto meta = metadata;
        meta.insert("helperVersion", "0.1.0");
        meta.insert("mode", "iso14443-4a.ppse");
        meta.insert("commands", "SELECT PPSE twice; no payment processing");
        recordingId_ = QString::fromStdString(store_->begin("nfc.iso14443-4a.ppse",
                              QJsonDocument::fromVariant(meta).toJson(QJsonDocument::Compact).toStdString()));
        auto request = jobs_.request("nfc.iso14443-4a.ppse", UFlipper::Control | UFlipper::DeviceApp | UFlipper::Nfc,
                                     UFlipper::Origin::Ui, true, true);
        if (request.decision != UFlipper::Decision::Granted)
            throw std::runtime_error("Device resources are not available");
        job_ = QString::fromStdString(request.id);
        active_ = true; state_ = "starting";
        issue(StartHelper);
        return true;
    } catch (const std::exception &error) {
        error_ = QString::fromUtf8(error.what());
        if (store_ && !recordingId_.isEmpty()) {
            try { store_->finish(recordingId_.toStdString(), UFlipper::CaptureState::Interrupted, error_.toStdString()); }
            catch (...) {}
        }
        store_.reset(); state_ = "interrupted";
        emit changed(); emit recordingSaved();
        return false;
    }
}
void NfcSession::record(const QByteArray &bytes, UFlipper::RecordType type) {
    const auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    if (!store_) throw std::runtime_error("Recording is unavailable");
    store_->append(recordingId_.toStdString(), bytes.constData(), size_t(bytes.size()), now, type);
    lastRecordNs_ = now;
}
void NfcSession::issue(int command, const QByteArray &payload) {
    if (!active_) return;
    pending_ = command; acknowledged_ = false; replied_ = false; reply_.clear();
    ++token_;
    QByteArray frame;
    if (command >= Info && command <= Deactivate) {
        frame = QByteArray(UF_NFC_HEADER, '\0');
        frame[0] = 'U'; frame[1] = 'F'; frame[2] = UF_NFC_VERSION;
        frame[3] = char(command); frame[4] = char(++sequence_);
        frame[6] = char(payload.size()); frame[7] = char(payload.size() >> 8);
        frame += payload;
    }
    try { record(frame.isEmpty() ? QByteArray(command == StartHelper ? "helper-start" : "helper-exit") : frame,
                 UFlipper::RecordType::Status); }
    catch (const std::exception &error) {
        if (error_.isEmpty()) error_ = tr("Aufzeichnung fehlgeschlagen: %1").arg(QString::fromUtf8(error.what()));
        if (command == StartHelper) { finalize(true); return; }
        if (command != ExitHelper) { stop(); return; }
        // The device must still receive Stop when the local disk has filled up.
    }
    deadline_.start(command == StartHelper ? 10000 : 5000);
    emit changed();
    emit commandRequested(command, frame, token_);
}
void NfcSession::acknowledge(quint64 token, const QString &error) {
    if (!active_ || token != token_) return;
    if (!error.isEmpty()) {
        if (pending_ == ExitHelper) {
            if (error_.isEmpty()) error_ = error;
            finalize(false);
        } else abort(error);
        return;
    }
    acknowledged_ = true;
    advance();
}
void NfcSession::helperState(bool started) {
    if (!active_) return;
    try { record(started ? "helper-started" : "helper-closed", UFlipper::RecordType::Status); }
    catch (const std::exception &error) { abort(QString::fromUtf8(error.what())); return; }
    if (started) started_ = true;
    else closed_ = true;
    if (!started && pending_ != ExitHelper) { abort(tr("Der NFC-Helfer wurde unerwartet beendet.")); return; }
    advance();
}
void NfcSession::receive(const QByteArray &frame) {
    if (!active_) return;
    try {
        record(frame, UFlipper::RecordType::Data);
        if (pending_ == ExitHelper) return; // A reply to the canceled command can arrive during Stop.
        const auto *b = reinterpret_cast<const uint8_t *>(frame.constData());
        if (frame.size() < 8 || frame.size() > UF_NFC_HEADER + UF_NFC_MAX_RESPONSE ||
            b[0] != 'U' || b[1] != 'F' || b[2] != UF_NFC_VERSION ||
            b[3] != (pending_ | 0x80) || b[4] != sequence_ ||
            uf_nfc_payload_size(b) != size_t(frame.size() - 8))
            throw std::runtime_error("Invalid or mismatched NFC helper response");
        replyStatus_ = b[5]; reply_ = frame.mid(8); replied_ = true;
        advance();
    } catch (const std::exception &error) { abort(QString::fromUtf8(error.what())); }
}
void NfcSession::advance() {
    if (!active_ || !acknowledged_) return;
    if (pending_ == StartHelper) {
        if (!started_) return;
        jobs_.markRunning(job_.toStdString());
        issue(Info); return;
    }
    if (pending_ == ExitHelper) {
        if (closed_) finalize(true);
        return;
    }
    if (!replied_) return;
    deadline_.stop();
    if (pending_ == Activate && replyStatus_ == UfNfcNoCard) {
        state_ = "searching"; retry_.start(); emit changed(); return;
    }
    if (replyStatus_ != UfNfcOk) { abort(tr("NFC-Helfer meldet Fehler %1.").arg(replyStatus_)); return; }
    try {
        if (pending_ == Info) {
            if (reply_ != "uf-nfc/0.1.0;iso14443-4a;short-apdu;max=240")
                throw std::runtime_error("Helper version or capabilities were not approved");
            state_ = "searching"; issue(Activate);
        } else if (pending_ == Activate) {
            readCard(reply_);
            state_ = "reading";
            issue(Transceive, QByteArray::fromHex("00a404000e325041592e5359532e444446303100"));
        } else if (pending_ == Transceive) {
            addResponse(reply_, sequence_);
            if (reply_.right(2) != QByteArray::fromHex("9000")) {
                abort(tr("PPSE-Verzeichnis nicht lesbar: %1.").arg(QString::fromLatin1(reply_.right(2).toHex())));
            } else if (responses_.size() < 2) {
                issue(Transceive, QByteArray::fromHex("00a404000e325041592e5359532e444446303100"));
            } else {
                state_ = "stopping";
                issue(Deactivate);
            }
        } else if (pending_ == Deactivate) stop();
    } catch (const std::exception &error) { abort(QString::fromUtf8(error.what())); }
}
void NfcSession::readCard(const QByteArray &payload) {
    if (payload.size() < 4 || uint8_t(payload[0]) > 10 || payload.size() != uint8_t(payload[0]) + 3)
        throw std::runtime_error("Invalid card activation metadata");
    const int length = uint8_t(payload[0]);
    const int limit = uint8_t(payload[length + 1]) | (uint8_t(payload[length + 2]) << 8);
    if (limit < 20 || limit > UF_NFC_MAX_APDU) throw std::runtime_error("Card frame limit cannot carry PPSE request");
    card_ = {{"protocol", "ISO14443-4A / ISO-DEP"}, {"uid", QString::fromLatin1(payload.mid(1, length).toHex(' '))},
             {"frameLimit", limit}};
}
void NfcSession::addResponse(const QByteArray &payload, int sequence, quint64 monotonicNs) {
    if (payload.size() < 2) throw std::runtime_error("APDU response lacks a status word");
    if (responses_.size() >= 2) return;
    responses_.append(QVariantMap{{"command", "SELECT PPSE"}, {"sequence", sequence},
        {"statusWord", QString::fromLatin1(payload.right(2).toHex()).toUpper()}, {"bytes", payload.size()},
        {"hex", QString::fromLatin1(payload.toHex(' '))},
        {"hostMonotonicNs", QString::number(monotonicNs ? monotonicNs : lastRecordNs_)}});
}
void NfcSession::stop() {
    if (!active_ || pending_ == ExitHelper) return;
    retry_.stop(); deadline_.stop(); state_ = "stopping";
    jobs_.requestStop(job_.toStdString());
    issue(ExitHelper);
}
void NfcSession::abort(const QString &reason) {
    if (error_.isEmpty()) error_ = reason;
    stop();
}
void NfcSession::rejectStart(const QString &reason) {
    if (!active_ || pending_ != StartHelper) return;
    error_ = reason;
    finalize(true); // The transport guarantees that no app start was sent.
}
void NfcSession::cancelBeforeStart() {
    if (!active_ || pending_ != ExitHelper) return;
    try { record("Canceled during preflight; helper start was not sent", UFlipper::RecordType::Status); }
    catch (const std::exception &error) { error_ = QString::fromUtf8(error.what()); }
    finalize(true);
}
void NfcSession::transportFailed(const QString &reason) {
    if (!active_) return;
    error_ = reason;
    jobs_.disconnectDevice(reason.toStdString());
    finalize(false);
}
void NfcSession::finalize(bool stopped) {
    deadline_.stop(); retry_.stop();
    stopConfirmed_ = stopped;
    uncertain_ = !stopped && online_;
    const bool complete = stopped && error_.isEmpty();
    try {
        if (store_ && !recordingId_.isEmpty()) {
            store_->finish(recordingId_.toStdString(), complete ? UFlipper::CaptureState::Completed : UFlipper::CaptureState::Interrupted,
                error_.isEmpty() ? (responses_.size() == 2 ? "PPSE read complete; helper stop confirmed" : "User stop confirmed") : error_.toStdString());
            verified_ = store_->verify(recordingId_.toStdString());
            if (!verified_) error_ = tr("Die Aufnahme konnte nicht verifiziert werden.");
        }
    } catch (const std::exception &error) { error_ = QString::fromUtf8(error.what()); verified_ = false; }
    if (!job_.isEmpty()) {
        if (stopped) { jobs_.requestStop(job_.toStdString()); jobs_.acknowledgeStop(job_.toStdString()); }
        else jobs_.fail(job_.toStdString(), error_.toStdString());
    }
    store_.reset(); active_ = false; pending_ = -1; ++token_;
    state_ = uncertain_ ? "uncertain" : !error_.isEmpty() ? "interrupted" : responses_.size() == 2 ? "completed" : "stopped";
    emit changed(); emit recordingSaved();
}
QUrl NfcSession::recordingUrl() const {
    static const QRegularExpression id("^[0-9a-f]{32}$");
    return id.match(recordingId_).hasMatch() ? QUrl::fromLocalFile(root_ + "/recordings/" + recordingId_) : QUrl();
}
void NfcSession::openRecording(const QString &id) {
    static const QRegularExpression valid("^[0-9a-f]{32}$");
    if (blocked() || !valid.match(id).hasMatch()) return;
    card_.clear(); responses_.clear(); verified_ = false; stopConfirmed_ = false; error_.clear();
    recordingId_ = id;
    try {
        UFlipper::CaptureStore store(root_.toStdString());
        if (!store.verify(id.toStdString())) throw std::runtime_error("Recording integrity check failed");
        QFile manifest(root_ + "/recordings/" + id + "/manifest.json");
        if (!manifest.open(QIODevice::ReadOnly)) throw std::runtime_error("Recording manifest cannot be opened");
        const auto object = QJsonDocument::fromJson(manifest.read(1024 * 1024)).toVariant().toMap();
        if (object.value("kind").toString() != "nfc.iso14443-4a.ppse")
            throw std::runtime_error("This recording is not an NFC PPSE session");
        QFile file(root_ + "/recordings/" + id + "/events.bin");
        if (!file.open(QIODevice::ReadOnly) || file.read(8) != "UFCAP001")
            throw std::runtime_error("Invalid recording format");
        while (!file.atEnd()) {
            const auto header = file.read(13);
            if (header.size() != 13) throw std::runtime_error("Truncated recording header");
            const auto length = qFromLittleEndian<quint32>(reinterpret_cast<const uchar *>(header.constData() + 9));
            if (length > 1024 * 1024) throw std::runtime_error("Recording frame exceeds limit");
            const auto bytes = file.read(length);
            if (bytes.size() != length) throw std::runtime_error("Truncated recording data");
            if (header[0] != char(UFlipper::RecordType::Data) || bytes.size() < 8) continue;
            const auto *b = reinterpret_cast<const uint8_t *>(bytes.constData());
            if (b[0] != 'U' || b[1] != 'F' || b[2] != 1 || uf_nfc_payload_size(b) != size_t(bytes.size()-8))
                throw std::runtime_error("Invalid saved NFC response");
            if (b[5] != UfNfcOk) continue;
            if (b[3] == (Activate | 0x80)) readCard(bytes.mid(8));
            if (b[3] == (Transceive | 0x80)) addResponse(bytes.mid(8), b[4],
                qFromLittleEndian<quint64>(reinterpret_cast<const uchar *>(header.constData() + 1)));
        }
        verified_ = true;
        state_ = "saved";
        error_ = object.value("state").toString() == "interrupted" ? object.value("reason").toString() : QString();
    } catch (const std::exception &error) { card_.clear(); responses_.clear(); error_ = QString::fromUtf8(error.what()); state_ = "interrupted"; }
    emit changed();
}
