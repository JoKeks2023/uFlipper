#pragma once
#include <QDateTime>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QVariantMap>
namespace Flipper {
class DeviceRegistry;
class FlipperZero;
} // namespace Flipper
class DeviceTelemetry : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantMap snapshot READ snapshot NOTIFY snapshotChanged)
  public:
    explicit DeviceTelemetry(Flipper::DeviceRegistry *registry, QObject *parent = nullptr);
    QVariantMap snapshot() const;
    void setPollingEnabled(bool enabled);
    Q_INVOKABLE void refresh();
  signals:
    void snapshotChanged();

  private:
    void attachDevice();
    void connectionChanged();
    void refreshStorage();
    Flipper::DeviceRegistry *registry_;
    QPointer<Flipper::FlipperZero> device_;
    QList<QMetaObject::Connection> connections_;
    QTimer timer_;
    QVariantMap power_, storage_;
    QDateTime observedAt_;
    QString error_;
    quint64 generation_ = 0, requestSequence_ = 0;
    quint64 storageSequence_ = 0;
    int storagePending_ = 0;
    bool connected_ = false, pending_ = false, pollingEnabled_ = false;
};
