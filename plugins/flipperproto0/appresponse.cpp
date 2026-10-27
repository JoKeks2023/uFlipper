#include "appresponse.h"
bool AppStateResponse::started() const {
    return message().content.app_state_response.state == PB_App_AppState_APP_STARTED;
}
QByteArray AppDataResponse::data() const {
    const auto *bytes = message().content.app_data_exchange_request.data;
    return bytes ? QByteArray(reinterpret_cast<const char *>(bytes->bytes), bytes->size) : QByteArray();
}
