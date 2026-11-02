#pragma once
#include "protocol.h"
class ApplicationBackend;
namespace Mcp {
QJsonArray nativeTools();
ToolReply nativeCall(ApplicationBackend *, const QString &, const QJsonObject &);
}
