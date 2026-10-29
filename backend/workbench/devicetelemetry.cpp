#include "devicetelemetry.h"
#include "deviceregistry.h"
#include "flipperzero/devicestate.h"
#include "flipperzero/flipperzero.h"
#include "flipperzero/protobufsession.h"
#include "flipperzero/rpc/propertygetoperation.h"
#include "flipperzero/rpc/storageinfooperation.h"
#include "powersnapshot.h"
#include <memory>
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
    storagePending_ = 0;
    device_ = registry_->currentDevice();
    power_.clear();
    storage_.clear();
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
        storagePending_ = 0;
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
    power.insert("stale", !connected_ || !error_.isEmpty() || observedAt_.isNull() ||
                              observedAt_.msecsTo(QDateTime::currentDateTimeUtc()) > 30000);
    if (!observedAt_.isNull())
        power.insert("observedAt", observedAt_.toString(Qt::ISODateWithMs));
    power.insert("pending", pending_);
    power.insert("error", error_);
    result.insert("power", power);
    QVariantMap storage;
    for (const QString &path : {QStringLiteral("/int"), QStringLiteral("/ext")}) {
        auto item = storage_.value(path).toMap();
        const auto observed = item.value("observedAt").toDateTime();
        item.insert("stale", !connected_ || observed.isNull() || !item.value("error").toString().isEmpty() ||
                                 observed.msecsTo(QDateTime::currentDateTimeUtc()) > 30000);
        if (!observed.isNull())
            item.insert("observedAt", observed.toString(Qt::ISODateWithMs));
        storage.insert(path, item);
    }
    result.insert("storage", storage);
    return result;
}
void DeviceTelemetry::refresh() {
    if (!pollingEnabled_ || !connected_ || pending_ || !device_)
        return;
    refreshStorage();
    auto generation = generation_;
    auto sequence = ++requestSequence_;
    auto expected = device_;
    pending_ = true;
    auto *operation = device_->rpc()->propertyGet("pwrinfo");
    operation->setTimeout(3000);
    connect(operation, &AbstractOperation::finished, this, [this, operation, generation, sequence, expected] {
        if (generation != generation_ || sequence != requestSequence_ || expected != device_)
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
    connect(operation, &QObject::destroyed, this, [this, generation, sequence] {
        if (generation == generation_ && sequence == requestSequence_ && pending_) {
            pending_ = false;
            error_ = tr("Power query canceled before completion");
            emit snapshotChanged();
        }
    });
    emit snapshotChanged();
}

void DeviceTelemetry::setPollingEnabled(bool enabled) {
    if (enabled == pollingEnabled_) return;
    pollingEnabled_ = enabled;
    if (enabled)
        refresh();
}

void DeviceTelemetry::refreshStorage() {
    if (storagePending_ || !device_)
        return;
    const auto generation = generation_;
    const auto sequence = ++storageSequence_;
    storagePending_ = 2;
    for (const QByteArray path : {QByteArray("/int"), QByteArray("/ext")}) {
        auto *op = device_->rpc()->storageInfo(path);
        op->setTimeout(3000);
        auto completed = std::make_shared<bool>(false);
        connect(op, &AbstractOperation::finished, this, [this, op, path, generation, sequence, completed] {
            if (generation != generation_ || sequence != storageSequence_)
                return;
            *completed = true;
            --storagePending_;
            auto item = storage_.value(QString::fromLatin1(path)).toMap();
            if (op->isError())
                item.insert("error", op->errorString());
            else if (!op->isPresent()) {
                item.insert("available", false);
                item.insert("error", tr("Storage is absent or cannot be read"));
            } else if (op->sizeFree() > op->sizeTotal() || !op->sizeTotal())
                item.insert("error", tr("Invalid storage information"));
            else {
                item = {{"available", true},
                        {"totalBytes", double(op->sizeTotal())},
                        {"freeBytes", double(op->sizeFree())},
                        {"usedBytes", double(op->sizeTotal() - op->sizeFree())},
                        {"observedAt", QDateTime::currentDateTimeUtc()},
                        {"error", QString()}};
            }
            storage_.insert(QString::fromLatin1(path), item);
            emit snapshotChanged();
        });
        connect(op, &QObject::destroyed, this, [this, path, generation, sequence, completed] {
            if (*completed || generation != generation_ || sequence != storageSequence_)
                return;
            --storagePending_;
            auto item = storage_.value(QString::fromLatin1(path)).toMap();
            item.insert("error", tr("Storage query canceled before completion"));
            storage_.insert(QString::fromLatin1(path), item);
            emit snapshotChanged();
        });
    }
}
