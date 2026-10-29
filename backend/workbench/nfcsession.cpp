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
    // Idle/Running RPC transitions are not a new physical connection.
    if (online == online_ && identity == identity_) return;
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
bool NfcSession::begin(const QVariantMap &metadata, const QString &mode) {
    if (!available()) return false;
    if (mode != "ppse" && mode != "ndef" && mode != "reader") return false;
    mode_ = mode; readComplete_ = false; readerPowerOn_ = true; ndef_.reset(); ndefRecords_.clear(); responseLabel_.clear();
    recordingId_.clear(); job_.clear(); error_.clear(); card_.clear(); responses_.clear();
    verified_ = false; stopConfirmed_ = false; started_ = false; closed_ = false;
    try {
        store_ = std::make_unique<UFlipper::CaptureStore>(root_.toStdString());
        auto meta = metadata;
        meta.insert("helperVersion", "0.1.0");
        const QString kind = "nfc.iso14443-4a." + mode_;
        meta.insert("mode", "iso14443-4a." + mode_);
        meta.insert("commands", mode_ == "ppse" ? "SELECT PPSE twice; no payment processing" :
                    mode_ == "ndef" ? "Read-only Type 4 NDEF mapping 2; SELECT and READ BINARY" : "Local PC/SC client commands");
        recordingId_ = QString::fromStdString(store_->begin(kind.toStdString(),
                              QJsonDocument::fromVariant(meta).toJson(QJsonDocument::Compact).toStdString()));
        auto request = jobs_.request(kind.toStdString(), UFlipper::Control | UFlipper::DeviceApp | UFlipper::Nfc,
                                     mode_ == "reader" ? UFlipper::Origin::Reader : UFlipper::Origin::Ui, true, true);
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
            if (mode_ == "reader") {
                state_ = "ready"; pending_ = -1; emit changed();
            } else if (mode_ == "ndef") {
                ndef_ = std::make_unique<UFlipper::Type4NdefReader>(card_.value("frameLimit").toUInt());
                responseLabel_ = QString::fromStdString(ndef_->label());
                const auto command = ndef_->command(); state_ = "reading";
                issue(Transceive, QByteArray(reinterpret_cast<const char *>(command.data()), int(command.size())));
            } else {
                state_ = "reading"; responseLabel_ = "SELECT PPSE";
                issue(Transceive, QByteArray::fromHex("00a404000e325041592e5359532e444446303100"));
            }
        } else if (pending_ == Transceive) {
            addResponse(reply_, sequence_);
            if (mode_ == "reader") {
                const auto response = reply_; state_ = "ready"; pending_ = -1;
                emit readerResponse(response); emit changed();
            } else if (mode_ == "ndef") {
                ndef_->accept(UFlipper::NfcBytes(reply_.begin(), reply_.end()));
                if (ndef_->complete()) {
                    readNdefRecords(); readComplete_ = true; state_ = "stopping"; issue(Deactivate);
                } else {
                    responseLabel_ = QString::fromStdString(ndef_->label());
                    const auto command = ndef_->command();
                    issue(Transceive, QByteArray(reinterpret_cast<const char *>(command.data()), int(command.size())));
                }
            } else if (reply_.right(2) != QByteArray::fromHex("9000")) {
                abort(tr("PPSE-Verzeichnis nicht lesbar: %1.").arg(QString::fromLatin1(reply_.right(2).toHex())));
            } else if (responses_.size() < 2) {
                issue(Transceive, QByteArray::fromHex("00a404000e325041592e5359532e444446303100"));
            } else {
                readComplete_ = true; state_ = "stopping";
                issue(Deactivate);
            }
        } else if (pending_ == Deactivate) {
            if (mode_ == "reader") {
                card_.clear();
                if (readerPowerOn_) { state_ = "searching"; issue(Activate); }
                else { state_ = "powered-off"; pending_ = -1; emit changed(); }
            } else stop();
        }
    } catch (const std::exception &error) { abort(QString::fromUtf8(error.what())); }
}
void NfcSession::readCard(const QByteArray &payload) {
    if (payload.size() < 4 || uint8_t(payload[0]) > 10 || payload.size() != uint8_t(payload[0]) + 3)
        throw std::runtime_error("Invalid card activation metadata");
    const int length = uint8_t(payload[0]);
    const int limit = uint8_t(payload[length + 1]) | (uint8_t(payload[length + 2]) << 8);
    if (limit < (mode_ == "ppse" ? 20 : 13) || limit > UF_NFC_MAX_APDU)
        throw std::runtime_error("Card frame limit cannot carry selected mode");
    card_ = {{"protocol", "ISO14443-4A / ISO-DEP"}, {"uid", QString::fromLatin1(payload.mid(1, length).toHex(' '))},
             {"frameLimit", limit}};
}
void NfcSession::addResponse(const QByteArray &payload, int sequence, quint64 monotonicNs) {
    if (payload.size() < 2) throw std::runtime_error("APDU response lacks a status word");
    if (mode_ == "ppse" && responses_.size() >= 2) return;
    if (responses_.size() >= 32) responses_.removeFirst();
    responses_.append(QVariantMap{{"command", responseLabel_.isEmpty() ? "PC/SC APDU" : responseLabel_}, {"sequence", sequence},
        {"statusWord", QString::fromLatin1(payload.right(2).toHex()).toUpper()}, {"bytes", payload.size()},
        {"hex", QString::fromLatin1(payload.toHex(' '))},
        {"hostMonotonicNs", QString::number(monotonicNs ? monotonicNs : lastRecordNs_)}});
}
void NfcSession::readNdefRecords() {
    ndefRecords_.clear();
    for (const auto &record : UFlipper::parseNdef(ndef_->message())) {
        const QByteArray type(reinterpret_cast<const char *>(record.type.data()), int(record.type.size()));
        const QByteArray data(reinterpret_cast<const char *>(record.payload.data()), int(record.payload.size()));
        QString content;
        if (record.tnf == 1 && type == "T" && !data.isEmpty()) {
            const auto status = uint8_t(data[0]); const int language = status & 0x3f;
            if (language + 1 <= data.size()) {
                const auto text = data.mid(1 + language);
                if (!(status & 0x80)) content = QString::fromUtf8(text);
                else if (text.size() % 2 == 0) {
                    const bool little = text.startsWith(QByteArray::fromHex("fffe"));
                    const int offset = little || text.startsWith(QByteArray::fromHex("feff")) ? 2 : 0;
                    std::u16string chars;
                    for (int i=offset;i<text.size();i+=2) chars.push_back(little ? uint8_t(text[i]) | (uint8_t(text[i+1])<<8) : (uint8_t(text[i])<<8) | uint8_t(text[i+1]));
                    content = QString::fromStdU16String(chars);
                }
            }
        } else if (record.tnf == 1 && type == "U" && !data.isEmpty()) {
            static const char *const prefixes[] = {"", "http://www.", "https://www.", "http://", "https://", "tel:", "mailto:", "ftp://anonymous:anonymous@", "ftp://", "ftps://", "sftp://", "smb://", "nfs://", "ftp://", "dav://", "news:", "telnet://", "imap:", "rtsp://", "urn:", "pop:", "sip:", "sips:", "tftp:", "btspp://", "btl2cap://", "btgoep://", "tcpobex://", "irdaobex://", "file://", "urn:epc:id:", "urn:epc:tag:", "urn:epc:pat:", "urn:epc:raw:", "urn:epc:", "urn:nfc:"};
            if (uint8_t(data[0]) < sizeof(prefixes)/sizeof(prefixes[0]))
                content = QString::fromLatin1(prefixes[uint8_t(data[0])]) + QString::fromUtf8(data.mid(1));
        }
        ndefRecords_.append(QVariantMap{{"tnf", record.tnf}, {"type", QString::fromUtf8(type)}, {"bytes", data.size()},
            {"content", content.left(4096)}, {"hex", QString::fromLatin1(data.left(256).toHex(' '))},
            {"previewTruncated", data.size() > 256 || content.size() > 4096}});
    }
}
bool NfcSession::readerTransmit(const QByteArray &apdu) {
    if (!readerReady() || apdu.size() < 4 || apdu.size() > card_.value("frameLimit").toInt()) return false;
    responseLabel_ = "PC/SC APDU"; state_ = "reading"; issue(Transceive, apdu); return true;
}
bool NfcSession::readerPower(bool on, bool reset) {
    if (!active_ || mode_ != "reader") return false;
    if (state_ == "powered-off") {
        if (on) { readerPowerOn_ = true; state_ = "searching"; issue(Activate); }
        return true;
    }
    if (readerReady()) {
        if (!on || reset) { readerPowerOn_ = on; state_ = "stopping-card"; issue(Deactivate); }
        return true;
    }
    return on && !reset && state_ == "searching";
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
                error_.isEmpty() ? (readComplete_ ? (mode_ == "ndef" ? "NDEF read complete; helper stop confirmed" : "PPSE read complete; helper stop confirmed") : "User stop confirmed") : error_.toStdString());
            verified_ = store_->verify(recordingId_.toStdString());
            if (!verified_) error_ = tr("Die Aufnahme konnte nicht verifiziert werden.");
        }
    } catch (const std::exception &error) { error_ = QString::fromUtf8(error.what()); verified_ = false; }
    if (!job_.isEmpty()) {
        if (stopped) { jobs_.requestStop(job_.toStdString()); jobs_.acknowledgeStop(job_.toStdString()); }
        else jobs_.fail(job_.toStdString(), error_.toStdString());
    }
    store_.reset(); active_ = false; pending_ = -1; ++token_;
    state_ = uncertain_ ? "uncertain" : !error_.isEmpty() ? "interrupted" : readComplete_ ? "completed" : "stopped";
    emit changed(); emit recordingSaved();
}
QUrl NfcSession::recordingUrl() const {
    static const QRegularExpression id("^[0-9a-f]{32}$");
    return id.match(recordingId_).hasMatch() ? QUrl::fromLocalFile(root_ + "/recordings/" + recordingId_) : QUrl();
}
void NfcSession::openRecording(const QString &id) {
    static const QRegularExpression valid("^[0-9a-f]{32}$");
    if (blocked() || !valid.match(id).hasMatch()) return;
    card_.clear(); responses_.clear(); ndefRecords_.clear(); ndef_.reset(); responseLabel_.clear();
    verified_ = false; stopConfirmed_ = false; error_.clear();
    recordingId_ = id;
    try {
        UFlipper::CaptureStore store(root_.toStdString());
        if (!store.verify(id.toStdString())) throw std::runtime_error("Recording integrity check failed");
        QFile manifest(root_ + "/recordings/" + id + "/manifest.json");
        if (!manifest.open(QIODevice::ReadOnly)) throw std::runtime_error("Recording manifest cannot be opened");
        const auto object = QJsonDocument::fromJson(manifest.read(1024 * 1024)).toVariant().toMap();
        const auto kind = object.value("kind").toString();
        if (kind != "nfc.iso14443-4a.ppse" && kind != "nfc.iso14443-4a.ndef" && kind != "nfc.iso14443-4a.reader")
            throw std::runtime_error("This recording is not a supported NFC session");
        mode_ = kind.section('.', -1);
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
            if (b[3] == (Activate | 0x80)) {
                readCard(bytes.mid(8));
                if (mode_ == "ndef") ndef_ = std::make_unique<UFlipper::Type4NdefReader>(card_.value("frameLimit").toUInt());
            }
            if (b[3] == (Transceive | 0x80)) {
                responseLabel_ = mode_ == "ppse" ? "SELECT PPSE" : mode_ == "ndef" && ndef_ ? QString::fromStdString(ndef_->label()) : "PC/SC APDU";
                addResponse(bytes.mid(8), b[4], qFromLittleEndian<quint64>(reinterpret_cast<const uchar *>(header.constData() + 1)));
                if (mode_ == "ndef" && ndef_) {
                    try {
                        ndef_->accept(UFlipper::NfcBytes(bytes.begin()+8, bytes.end()));
                        if (ndef_->complete()) readNdefRecords();
                    } catch (const std::exception &) {
                        if (object.value("state").toString() != "interrupted") throw;
                        // A failed card read is still an intact original recording.
                        ndef_.reset(); ndefRecords_.clear();
                    }
                }
            }
        }
        verified_ = true;
        state_ = "saved";
        error_ = object.value("state").toString() == "interrupted" ? object.value("reason").toString() : QString();
    } catch (const std::exception &error) { card_.clear(); responses_.clear(); ndefRecords_.clear(); error_ = QString::fromUtf8(error.what()); state_ = "interrupted"; }
    emit changed();
}
