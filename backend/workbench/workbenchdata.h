#pragma once
#include <QObject>
#include <QUrl>
#include <QVariantList>
class WorkbenchData : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList recordings READ recordings NOTIFY changed)
    Q_PROPERTY(QVariantList backups READ backups NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(QUrl storageUrl READ storageUrl CONSTANT)
  public:
    explicit WorkbenchData(QObject *parent = nullptr);
    QVariantList recordings() const {
        return recordings_;
    }
    QVariantList backups() const {
        return backups_;
    }
    QString error() const {
        return error_;
    }
    QUrl storageUrl() const;
    Q_INVOKABLE void refresh();
    Q_INVOKABLE QUrl recordingUrl(const QString &id) const;
    Q_INVOKABLE QUrl backupUrl(const QString &id) const;
  signals:
    void changed();

  private:
    QString root_, error_;
    QVariantList recordings_, backups_;
};
