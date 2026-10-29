#pragma once
#include "../../workbench/core/capture_store.h"
#include "../../workbench/core/operation_coordinator.h"
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>
#include <memory>
#include "../../workbench/core/ndef.h"

// Protocol/session state is independent of the Qt serial transport. Commands use
// the existing application connection; tests can supply acknowledgements directly.
class NfcSession : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString mode READ mode NOTIFY changed)
    Q_PROPERTY(QVariantList ndefRecords READ ndefRecords NOTIFY changed)
    Q_PROPERTY(bool active READ active NOTIFY changed)
    Q_PROPERTY(bool available READ available NOTIFY changed)
    Q_PROPERTY(bool blocked READ blocked NOTIFY changed)
    Q_PROPERTY(QString state READ state NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(QString recordingId READ recordingId NOTIFY changed)
    Q_PROPERTY(bool recordingVerified READ recordingVerified NOTIFY changed)
    Q_PROPERTY(bool stopConfirmed READ stopConfirmed NOTIFY changed)
    Q_PROPERTY(QVariantMap card READ card NOTIFY changed)
    Q_PROPERTY(QVariantList responses READ responses NOTIFY changed)
  public:
    enum Command { StartHelper = 0, Info = 1, Activate = 2, Transceive = 3, Deactivate = 4, ExitHelper = 5 };
    Q_ENUM(Command)
    explicit NfcSession(const QString &root, QObject *parent = nullptr);
    ~NfcSession() override;
    QString mode() const { return mode_; }
    QVariantList ndefRecords() const { return ndefRecords_; }
    bool readerReady() const { return active_ && mode_ == "reader" && state_ == "ready" && pending_ == -1; }
    bool readerTransmit(const QByteArray &apdu);
    bool readerPower(bool on, bool reset = false);
    bool active() const { return active_; }
    bool available() const { return online_ && !active_ && !uncertain_; }
    bool blocked() const { return active_ || uncertain_; }
    QString state() const { return state_; }
    QString error() const { return error_; }
    QString recordingId() const { return recordingId_; }
    bool recordingVerified() const { return verified_; }
    bool stopConfirmed() const { return stopConfirmed_; }
    QVariantMap card() const { return card_; }
    QVariantList responses() const { return responses_; }
    quint64 commandToken() const { return token_; }
    void setConnection(const QString &identity, bool online);
    bool begin(const QVariantMap &metadata, const QString &mode = "ppse");
    void acknowledge(quint64 token, const QString &error = {});
    void helperState(bool started);
    void receive(const QByteArray &frame);
    void rejectStart(const QString &reason);
    void cancelBeforeStart();
    void transportFailed(const QString &reason);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void openRecording(const QString &id);
    Q_INVOKABLE QUrl recordingUrl() const;
  signals:
    void changed();
    void commandRequested(int command, const QByteArray &frame, quint64 token);
    void recordingSaved();
    void readerResponse(const QByteArray &response);
  private:
    void issue(int command, const QByteArray &payload = {});
    void advance();
    void abort(const QString &reason);
    void finalize(bool stopped);
    void record(const QByteArray &bytes, UFlipper::RecordType type);
    void readCard(const QByteArray &payload);
    void readNdefRecords();
    void addResponse(const QByteArray &payload, int sequence, quint64 monotonicNs = 0);
    QString root_, identity_, recordingId_, job_, state_ = QStringLiteral("idle"), error_;
    QVariantMap card_;
    QVariantList responses_, ndefRecords_;
    QString mode_ = "ppse", responseLabel_;
    std::unique_ptr<UFlipper::Type4NdefReader> ndef_;
    bool readComplete_ = false, readerPowerOn_ = true;
    UFlipper::OperationCoordinator jobs_;
    std::unique_ptr<UFlipper::CaptureStore> store_;
    QTimer deadline_, retry_;
    quint64 token_ = 0, lastRecordNs_ = 0;
    uint8_t sequence_ = 0;
    int pending_ = -1;
    bool online_ = false, active_ = false, uncertain_ = false, verified_ = false;
    bool stopConfirmed_ = false, acknowledged_ = false, replied_ = false, started_ = false, closed_ = false;
    QByteArray reply_;
    uint8_t replyStatus_ = 0;
};
