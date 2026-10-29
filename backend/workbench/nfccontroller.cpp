#include "nfccontroller.h"
#include "viccbridge.h"
#include "appresponseinterface.h"
#include "deviceregistry.h"
#include "flipperzero/devicestate.h"
#include "flipperzero/flipperzero.h"
#include "flipperzero/protobufsession.h"
#include "flipperzero/rpc/appoperation.h"
#include "flipperzero/rpc/storagereadoperation.h"
#include "flipperzero/rpc/storagestatoperation.h"
#include "flipperzero/screenstreamer.h"
#include <QBuffer>
#include <QCryptographicHash>
#include <QDateTime>
#include <QStandardPaths>
#include <memory>

NfcController::NfcController(Flipper::DeviceRegistry *registry, Flipper::Zero::ScreenStreamer *screen,
                             std::function<bool()> idle, QObject *parent)
    : NfcSession(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/uFlipper/workbench", parent),
      registry_(registry), screen_(screen), idle_(std::move(idle)) {
    connect(registry_, &Flipper::DeviceRegistry::currentDeviceChanged, this, &NfcController::attach);
    connect(this, &NfcSession::commandRequested, this, &NfcController::dispatch);
    reader_ = new ViccBridge(*this, this);
    connect(reader_, &ViccBridge::changed, this, &NfcSession::changed);
    attach();
}
bool NfcController::approvedHelperTarget(const QString &commit, const QString &target) {
    // DeviceInfo has already converted the firmware's numeric target to "f7".
    return commit == "e1784e74" && target == "f7";
}
bool NfcController::canStart() const {
    return available() && idle_() && device_ && device_->deviceState()->isOnline() &&
           !device_->deviceState()->isRecoveryMode() && device_->rpc()->isSessionUp();
}
void NfcController::attach() {
    for (const auto &connection : connections_) QObject::disconnect(connection);
    connections_.clear();
    device_ = registry_->currentDevice();
    helperStartSent_ = false;
    if (device_) {
        connections_ << connect(device_->deviceState(), &Flipper::Zero::DeviceState::isOnlineChanged,
                                this, &NfcController::connectionChanged);
        connections_ << connect(device_->rpc(), &Flipper::Zero::ProtobufSession::sessionStateChanged,
                                this, &NfcController::connectionChanged);
        connections_ << connect(device_->rpc(), &Flipper::Zero::ProtobufSession::broadcastResponseReceived,
                                this, [this](QObject *message) {
            if (!active() || !helperStartSent_) return;
            if (auto *state = qobject_cast<AppStateResponseInterface *>(message)) helperState(state->started());
            else if (auto *data = qobject_cast<AppDataResponseInterface *>(message)) receive(data->data());
        });
    }
    connectionChanged();
}
void NfcController::connectionChanged() {
    const bool online = device_ && device_->deviceState()->isOnline() && device_->rpc()->isSessionUp();
    setConnection(device_ ? device_->deviceState()->deviceInfo().systemLocation : QString(), online);
    if (!online) helperStartSent_ = false;
}
QString NfcController::readerStatus() const { return reader_->status(); }
void NfcController::start(const QString &mode) {
    if (!canStart() || (mode != "ppse" && mode != "ndef" && mode != "reader")) return;
#ifndef Q_OS_MACOS
    if (mode == "reader") return;
#endif
    if (mode == "reader" && !reader_->start()) return;
    const auto &info = device_->deviceState()->deviceInfo();
    helperStartSent_ = false;
    const bool begun = begin({{"deviceName", info.name}, {"firmwareVersion", info.firmware.version},
           {"firmwareCommit", info.firmware.commit}, {"hardwareTarget", info.hardware.target},
           {"rpcMajor", info.protobuf.versionMajor}, {"rpcMinor", info.protobuf.versionMinor},
           {"startedAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
           {"helperSha256", "ab0bcae111b085a8cf79ef9c5dd423331f0ae86519228cb0ed79624470e0d632"}}, mode);
    if (!begun && mode == "reader") reader_->stop();
}
void NfcController::dispatch(int command, const QByteArray &frame, quint64 token) {
    if (!device_ || !device_->rpc()->isSessionUp()) {
        transportFailed(tr("Die Geräteverbindung ist nicht verfügbar.")); return;
    }
    if (command == StartHelper) {
        screen_->stop();
        checkHelper(token);
    } else if (command == ExitHelper) {
        if (!helperStartSent_) {
            cancelBeforeStart(); // No app start was sent; do not fabricate a device broadcast.
        } else watch(device_->rpc()->appExit(), token);
    } else watch(device_->rpc()->appExchange(frame), token);
}
void NfcController::checkHelper(quint64 token) {
    const auto &info = device_->deviceState()->deviceInfo();
    if (!approvedHelperTarget(info.firmware.commit, info.hardware.target)) {
        rejectStart(tr("Dieser Helfer ist bisher nur für Momentum mntm-012 (e1784e74), Ziel f7 freigegeben."));
        return;
    }
    const auto expected = device_;
    auto *stat = device_->rpc()->storageStat("/ext/apps/Tools/uf_nfc_bridge.fap");
    stat->setTimeout(3000);
    connect(stat, &AbstractOperation::finished, this, [this, stat, token, expected] {
        if (!active() || commandToken() != token || device_ != expected) return;
        if (stat->isError() || !stat->hasFile() || stat->size() != 6080) {
            rejectStart(tr("Der freigegebene NFC-Helfer 0.1.0 fehlt oder wurde verändert. Keine automatische Installation."));
            return;
        }
        auto buffer = std::make_shared<QBuffer>();
        buffer->open(QIODevice::WriteOnly);
        auto *read = device_->rpc()->storageRead("/ext/apps/Tools/uf_nfc_bridge.fap", buffer.get());
        read->setTimeout(4000);
        connect(read, &AbstractOperation::finished, this, [this, read, buffer, token, expected] {
            if (!active() || commandToken() != token || device_ != expected) return;
            const auto hash = QCryptographicHash::hash(buffer->data(), QCryptographicHash::Sha256).toHex();
            if (read->isError() || buffer->size() != 6080 ||
                hash != "ab0bcae111b085a8cf79ef9c5dd423331f0ae86519228cb0ed79624470e0d632") {
                rejectStart(tr("Die Prüfsumme des NFC-Helfers stimmt nicht mit der Freigabe überein."));
                return;
            }
            helperStartSent_ = true;
            watch(device_->rpc()->appStart("/ext/apps/Tools/uf_nfc_bridge.fap"), token);
        });
    });
}
void NfcController::watch(AbstractOperation *operation, quint64 token) {
    operation->setTimeout(4000);
    auto finished = std::make_shared<bool>(false);
    connect(operation, &AbstractOperation::finished, this, [this, operation, token, finished] {
        *finished = true;
        acknowledge(token, operation->isError() ? operation->errorString() : QString());
    });
    connect(operation, &QObject::destroyed, this, [this, token, finished] {
        if (!*finished) acknowledge(token, tr("Die Geräteaktion wurde vor ihrer Bestätigung abgebrochen."));
    });
}
