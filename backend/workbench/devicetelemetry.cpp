#include "devicetelemetry.h"
#include "deviceregistry.h"
#include "flipperzero/devicestate.h"
#include "flipperzero/flipperzero.h"
#include "flipperzero/protobufsession.h"
#include "flipperzero/rpc/propertygetoperation.h"
#include "powersnapshot.h"
DeviceTelemetry::DeviceTelemetry(Flipper::DeviceRegistry *registry, QObject *parent)
    : QObject(parent), registry_(registry) {
    connect(registry_, &Flipper::DeviceRegistry::currentDeviceChanged, this, &DeviceTelemetry::attachDevice);
    timer_.setInterval(10000);
    connect(&timer_, &QTimer::timeout, this, [this] {
        refresh();
        emit snapshotChanged();
    });
    timer_.start();
    attachDevice();
}
void DeviceTelemetry::attachDevice() {
    for (const auto &connection : connections_)
        QObject::disconnect(connection);
    connections_.clear();
    ++generation_;
    pending_ = false;
    device_ = registry_->currentDevice();
    power_.clear();
    observedAt_ = {};
    error_.clear();
    connected_ = false;
    if (device_) {
        connections_ << connect(device_->deviceState(), &Flipper::Zero::DeviceState::isOnlineChanged, this,
                                &DeviceTelemetry::connectionChanged);
        connections_ << connect(device_->deviceState(), &Flipper::Zero::DeviceState::deviceInfoChanged, this,
                                &DeviceTelemetry::snapshotChanged);
        connections_ << connect(device_->rpc(), &Flipper::Zero::ProtobufSession::sessionStateChanged, this,
                                &DeviceTelemetry::connectionChanged);
        connections_ << connect(device_, &QObject::destroyed, this, [this] { connectionChanged(); });
    }
    connectionChanged();
    emit snapshotChanged();
}
void DeviceTelemetry::connectionChanged() {
    bool online = device_ && device_->deviceState()->isOnline() && device_->rpc()->isSessionUp();
    if (online != connected_) {
        connected_ = online;
        ++generation_;
        pending_ = false;
        emit snapshotChanged();
        if (online)
            QTimer::singleShot(0, this, &DeviceTelemetry::refresh);
    }
}
QVariantMap DeviceTelemetry::snapshot() const {
    QVariantMap result{{"connected", connected_}};
    if (device_) {
        const auto &info = device_->deviceState()->deviceInfo();
        result.insert("device", QVariantMap{{"name", info.name},
                                            {"model", info.model},
                                            {"firmwareVersion", info.firmware.version},
                                            {"firmwareCommit", info.firmware.commit},
                                            {"firmwareBranch", info.firmware.branch},
                                            {"hardwareTarget", info.hardware.target},
                                            {"rpcMajor", info.protobuf.versionMajor},
                                            {"rpcMinor", info.protobuf.versionMinor}});
    }
    QVariantMap power = power_;
    power.insert("available", !observedAt_.isNull());
    power.insert("stale", !connected_ || observedAt_.isNull() ||
                              observedAt_.msecsTo(QDateTime::currentDateTimeUtc()) > 30000);
    if (!observedAt_.isNull())
        power.insert("observedAt", observedAt_.toString(Qt::ISODateWithMs));
    power.insert("pending", pending_);
    power.insert("error", error_);
    result.insert("power", power);
    return result;
}
void DeviceTelemetry::refresh() {
    if (!pollingEnabled_ || !connected_ || pending_ || !device_)
        return;
    auto generation = generation_;
    auto expected = device_;
    pending_ = true;
    auto *operation = device_->rpc()->propertyGet("pwrinfo");
    operation->setTimeout(3000);
    connect(operation, &AbstractOperation::finished, this, [this, operation, generation, expected] {
        if (generation != generation_ || expected != device_)
            return;
        pending_ = false;
        if (operation->isError())
            error_ = operation->errorString();
        else {
            auto power = UFlipper::powerSnapshot(operation->values());
            if (power.value("formatSupported").toBool()) {
                power_ = power;
                observedAt_ = QDateTime::currentDateTimeUtc();
                error_.clear();
            } else
                error_ = tr("Power information has an unsupported or missing format");
        }
        emit snapshotChanged();
    });
    emit snapshotChanged();
}

void DeviceTelemetry::setPollingEnabled(bool enabled) {
    pollingEnabled_ = enabled;
    if (enabled)
        refresh();
}
