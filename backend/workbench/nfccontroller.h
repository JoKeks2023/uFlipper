#pragma once
#include "nfcsession.h"
#include <QPointer>
#include <functional>
namespace Flipper { class DeviceRegistry; class FlipperZero; namespace Zero { class ScreenStreamer; } }
class AbstractOperation;
class NfcController : public NfcSession {
    Q_OBJECT
    Q_PROPERTY(bool canStart READ canStart NOTIFY changed)
  public:
    NfcController(Flipper::DeviceRegistry *registry, Flipper::Zero::ScreenStreamer *screen,
                  std::function<bool()> idle, QObject *parent = nullptr);
    bool canStart() const;
    Q_INVOKABLE void start();
    void conditionsChanged() { emit changed(); }
  private:
    void attach();
    void connectionChanged();
    void dispatch(int command, const QByteArray &frame, quint64 token);
    void checkHelper(quint64 token);
    void watch(AbstractOperation *operation, quint64 token);
    Flipper::DeviceRegistry *registry_;
    Flipper::Zero::ScreenStreamer *screen_;
    std::function<bool()> idle_;
    QPointer<Flipper::FlipperZero> device_;
    QList<QMetaObject::Connection> connections_;
    bool helperStartSent_ = false;
};
