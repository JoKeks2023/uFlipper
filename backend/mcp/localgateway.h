#pragma once
#include "protocol.h"
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <QHash>
#include <QTimer>
#include <memory>

namespace Mcp {
QString localDirectory();
QString machineIdentity();
QJsonObject readPrivateJson(const QString &path);
class LocalGateway : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(QStringList clients READ clients NOTIFY changed)
    Q_PROPERTY(QString command READ command CONSTANT)
 public:
    LocalGateway(QJsonArray tools, Protocol::Call call, std::function<void()> revoked,
                 QString directory, QString machine, QObject *parent = nullptr);
    ~LocalGateway() override;
    bool enabled() const { return server_.isListening(); }
    QString error() const { return error_; }
    QStringList clients() const;
    QString command() const;
    Q_INVOKABLE void grant();
    Q_INVOKABLE void revoke();
 signals:
    void changed();
 private:
    struct Client {
        Client(QJsonArray tools, Protocol::Call call) : protocol(std::move(tools), std::move(call)) {}
        QByteArray input;
        Protocol protocol;
        bool authenticated = false;
    };
    bool start();
    void accept();
    void receive(QLocalSocket *socket);
    void closeClients();
    bool save(const QString &name, const QJsonObject &value);
    QLocalServer server_;
    QHash<QLocalSocket *, std::shared_ptr<Client>> clients_;
    QJsonArray tools_;
    Protocol::Call call_;
    std::function<void()> revoked_;
    QString directory_, machine_, token_, error_;
};
}
