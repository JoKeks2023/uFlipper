#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QPointer>
#include <QTimer>
#include <QQueue>
class NfcSession;
// vsmartcard VICC endpoint, reverse mode: the OS driver connects to loopback.
// This adapter never opens a serial port and does not manufacture card replies.
class ViccBridge : public QObject {
    Q_OBJECT
 public:
    explicit ViccBridge(NfcSession &session, QObject *parent = nullptr);
    bool start(quint16 port = 35963);
    void stop();
    QString status() const;
    QString error() const { return error_; }
    quint16 port() const { return server_.serverPort(); }
 signals:
    void changed();
 private:
    void acceptClient();
    void receive();
    void process();
    void send(const QByteArray &payload);
    void fail(const QString &reason);
    NfcSession &session_;
    QTcpServer server_;
    QPointer<QTcpSocket> client_;
    QByteArray input_;
    QQueue<QByteArray> queue_;
    QTimer deadline_, activationWait_;
    QString error_;
    bool sessionSeen_ = false, waitingResponse_ = false, processing_ = false, powerPending_ = false;
};
