#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <functional>

namespace Mcp {
constexpr qsizetype MaxMessage = 64 * 1024;
constexpr qsizetype MaxBuffered = 256 * 1024;
struct ToolReply { QJsonObject data; bool error = false; };
// Per stdio client. Modern requests carry their own version/capabilities;
// legacy requests use initialize + notifications/initialized.
class Protocol {
 public:
    using Call = std::function<ToolReply(const QString &, const QJsonObject &)>;
    Protocol(QJsonArray tools, Call call);
    QJsonObject receive(const QByteArray &line);
    QString clientName() const { return client_; }
    static QJsonObject tool(const QString &, const QString &, QJsonObject properties = {},
                            QJsonArray required = {}, bool readOnly = true);
 private:
    QJsonObject result(const QJsonValue &, QJsonObject, bool modern) const;
    static QJsonObject error(const QJsonValue &, int, const QString &, QJsonObject data = {});
    QJsonArray tools_;
    Call call_;
    QString client_;
    bool initialized_ = false, ready_ = false;
};
}
