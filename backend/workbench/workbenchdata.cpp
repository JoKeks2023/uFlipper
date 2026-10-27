#include "workbenchdata.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTimer>
#include <memory>
#include <sqlite3.h>
#include <stdexcept>
namespace {
QVariantList readCatalog(const QString &path, const char *query) {
    if (!QFileInfo::exists(path))
        return {};
    sqlite3 *raw = nullptr;
    const int status = sqlite3_open_v2(path.toUtf8().constData(), &raw, SQLITE_OPEN_READONLY, nullptr);
    std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db(raw, sqlite3_close);
    if (status != SQLITE_OK)
        throw std::runtime_error("Local catalog cannot be opened");
    sqlite3_busy_timeout(raw, 50);
    sqlite3_stmt *statement = nullptr;
    if (sqlite3_prepare_v2(raw, query, -1, &statement, nullptr) != SQLITE_OK)
        throw std::runtime_error("Local catalog format cannot be read");
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> cleanup(statement, sqlite3_finalize);
    QVariantList rows;
    int step;
    while ((step = sqlite3_step(statement)) == SQLITE_ROW) {
        QVariantMap row;
        for (int i = 0; i < sqlite3_column_count(statement); ++i) {
            const auto name = QString::fromUtf8(sqlite3_column_name(statement, i));
            if (sqlite3_column_type(statement, i) == SQLITE_INTEGER)
                row.insert(name, qlonglong(sqlite3_column_int64(statement, i)));
            else {
                const auto *text = sqlite3_column_text(statement, i);
                row.insert(name, text ? QString::fromUtf8(reinterpret_cast<const char *>(text)) : QString());
            }
        }
        rows.append(row);
    }
    if (step != SQLITE_DONE)
        throw std::runtime_error("Local catalog read failed");
    return rows;
}
bool validId(const QString &id) {
    static const QRegularExpression expression(QStringLiteral("^[0-9a-f]{32}$"));
    return expression.match(id).hasMatch();
}
} // namespace
WorkbenchData::WorkbenchData(QObject *parent)
    : QObject(parent),
      root_(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/uFlipper/workbench") {
    QDir().mkpath(root_);
    QFile::setPermissions(root_, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    refresh();
    auto *timer = new QTimer(this);
    timer->setInterval(10000);
    connect(timer, &QTimer::timeout, this, &WorkbenchData::refresh);
    timer->start();
}
QUrl WorkbenchData::storageUrl() const {
    return QUrl::fromLocalFile(root_);
}
QUrl WorkbenchData::recordingUrl(const QString &id) const {
    return validId(id) ? QUrl::fromLocalFile(root_ + "/recordings/" + id) : QUrl();
}
QUrl WorkbenchData::backupUrl(const QString &id) const {
    return validId(id) ? QUrl::fromLocalFile(root_ + "/backups/" + id) : QUrl();
}
void WorkbenchData::refresh() {
    error_.clear();
    try {
        recordings_ =
            readCatalog(root_ + "/catalog.sqlite", "SELECT id,kind,state,bytes,reason,started_ms FROM "
                                                   "recordings ORDER BY started_ms DESC,id LIMIT 40");
        backups_ = readCatalog(
            root_ + "/backups/catalog.sqlite",
            "SELECT id,scope,state,reason,started_ms FROM backups ORDER BY started_ms DESC,id LIMIT 40");
    } catch (const std::exception &error) {
        recordings_.clear();
        backups_.clear();
        error_ = QString::fromUtf8(error.what());
    }
    emit changed();
}
