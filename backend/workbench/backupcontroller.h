#pragma once
#include "backupsession.h"
#include <QPointer>
#include <functional>
namespace Flipper { class DeviceRegistry; class FlipperZero; namespace Zero { class ScreenStreamer; } }
class AbstractOperation;
class BackupController : public BackupSession {
    Q_OBJECT
    Q_PROPERTY(bool canStart READ canStart NOTIFY changed)
    Q_PROPERTY(QString limitation READ limitation CONSTANT)
 public:
    BackupController(Flipper::DeviceRegistry *, Flipper::Zero::ScreenStreamer *, std::function<bool()> idle, QObject *parent = nullptr);
    bool canStart() const;
    QString limitation() const;
    Q_INVOKABLE void start();
    void conditionsChanged() { emit changed(); }
 private:
    void attach();
    void connectionChanged();
    void dispatch(int command, const QString &path, quint64 token);
    void watch(AbstractOperation *, quint64 token, std::function<void()> success);
    Flipper::DeviceRegistry *registry_;
    Flipper::Zero::ScreenStreamer *screen_;
    std::function<bool()> idle_;
    QPointer<Flipper::FlipperZero> device_;
    QList<QMetaObject::Connection> connections_;
};
