#pragma once
#include "abstractprotobufoperation.h"
namespace Flipper {
namespace Zero {
class AppOperation : public AbstractProtobufOperation {
    Q_OBJECT
  public:
    enum Action { Start, Exit, Exchange };
    AppOperation(uint32_t id, Action action, const QByteArray &payload, QObject *parent = nullptr);
    const QString description() const override;
    const QByteArray encodeRequest(ProtobufPluginInterface *encoder) override;

  private:
    bool processResponse(QObject *response) override;
    Action action_;
    QByteArray payload_;
};
} // namespace Zero
} // namespace Flipper
