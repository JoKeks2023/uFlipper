#pragma once
#include "../../workbench/core/backup_store.h"
#include "../../workbench/core/operation_coordinator.h"
#include <QObject>
#include <QIODevice>
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QMap>
#include <QQueue>
#include <QThread>
#include <QUrl>
#include <QVariantMap>
#include <memory>
struct BackupRemoteEntry { QString path; quint64 bytes = 0; bool directory = false; };
class BackupSession : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool active READ active NOTIFY changed)
    Q_PROPERTY(bool blocked READ blocked NOTIFY changed)
    Q_PROPERTY(QString state READ state NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(QString backupId READ backupId NOTIFY changed)
    Q_PROPERTY(QString currentPath READ currentPath NOTIFY changed)
    Q_PROPERTY(qulonglong bytesDone READ bytesDone NOTIFY changed)
    Q_PROPERTY(qulonglong bytesTotal READ bytesTotal NOTIFY changed)
    Q_PROPERTY(int filesDone READ filesDone NOTIFY changed)
    Q_PROPERTY(int filesTotal READ filesTotal NOTIFY changed)
    Q_PROPERTY(bool verified READ verified NOTIFY changed)
    Q_PROPERTY(bool canCancel READ canCancel NOTIFY changed)
 public:
    enum Command { StorageInfo, List, Read, Digest };
    Q_ENUM(Command)
    explicit BackupSession(QString root, QObject *parent = nullptr);
    ~BackupSession() override;
    bool active() const { return active_; }
    bool blocked() const { return active_ || uncertain_; }
    bool available() const { return online_ && !blocked(); }
    QString state() const { return state_; }
    QString error() const { return error_; }
    QString backupId() const { return id_; }
    QString currentPath() const { return current_; }
    qulonglong bytesDone() const { return bytesDone_; }
    qulonglong bytesTotal() const { return bytesTotal_; }
    int filesDone() const { return filesDone_; }
    int filesTotal() const { return files_.size(); }
    bool verified() const { return verified_; }
    bool canCancel() const { return active_ && state_ != "verifying" && !cancelled_; }
    void setConnection(const QString &identity, bool online);
    bool begin(const QVariantMap &metadata);
    void storageInfo(quint64 token, bool present);
    void listed(quint64 token, const QList<BackupRemoteEntry> &entries);
    void readFinished(quint64 token);
    void digested(quint64 token, const QByteArray &md5);
    void failed(quint64 token, const QString &reason, bool confirmed = false);
    QIODevice *sink() const;
    Q_INVOKABLE void cancel();
    Q_INVOKABLE QUrl backupUrl() const;
 signals:
    void changed();
    void commandRequested(int command, const QString &path, quint64 token);
    void transportStopRequested();
    void saved();
 private:
    class Sink;
    friend class Sink;
    qint64 write(const char *data, qint64 size);
    void issue(int command, const QString &path);
    void nextDirectory();
    void nextFile();
    void nextDigest();
    void startVerification();
    void finishError(const QString &reason, bool confirmed);
    void finalize(bool success, bool confirmed);
    bool accepts(quint64 token) const;
    QString root_, identity_, id_, current_, error_, state_ = "idle", job_;
    QMap<QString, BackupRemoteEntry> inventory_, finalInventory_;
    QMap<QString, QByteArray> digests_;
    QQueue<QString> directories_;
    QList<QString> files_;
    std::shared_ptr<UFlipper::BackupStore> store_;
    std::unique_ptr<Sink> sink_;
    UFlipper::OperationCoordinator jobs_;
    QCryptographicHash localMd5_{QCryptographicHash::Md5};
    QElapsedTimer paint_;
    QThread *verification_ = nullptr;
    quint64 token_ = 0, bytesDone_ = 0, bytesTotal_ = 0, currentBytes_ = 0;
    int pending_ = -1, filesDone_ = 0, rootIndex_ = 0, digestIndex_ = 0;
    bool active_ = false, online_ = false, uncertain_ = false, cancelled_ = false, finalPass_ = false, verified_ = false;
};
