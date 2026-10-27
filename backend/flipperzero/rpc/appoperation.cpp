#include "appoperation.h"
#include "mainresponseinterface.h"
#include "protobufplugininterface.h"
using namespace Flipper::Zero;
AppOperation::AppOperation(uint32_t id, Action action, const QByteArray &payload, QObject *parent)
    : AbstractProtobufOperation(id, parent), action_(action), payload_(payload) {}
const QString AppOperation::description() const {
    switch (action_) {
    case Start:
        return QStringLiteral("Start RPC application");
    case Exit:
        return QStringLiteral("Exit RPC application");
    case Exchange:
        return QStringLiteral("Exchange RPC application data");
    }
    return {};
}
const QByteArray AppOperation::encodeRequest(ProtobufPluginInterface *encoder) {
    switch (action_) {
    case Start:
        return encoder->appStart(id(), payload_);
    case Exit:
        return encoder->appExit(id());
    case Exchange:
        return encoder->appExchange(id(), payload_);
    }
    return {};
}
bool AppOperation::processResponse(QObject *response) {
    const auto *main = qobject_cast<MainResponseInterface *>(response);
    // App errors are not fatal to the RPC session, but they must fail this operation.
    return main && main->type() == MainResponseInterface::Empty && main->isOk();
}
