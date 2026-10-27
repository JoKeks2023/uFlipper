#include "apprequest.h"
#include <cstdlib>
#include <cstring>
#include <new>
AppStartRequest::AppStartRequest(uint32_t id, const QByteArray &path)
    : MainRequest(id, PB_Main_app_start_request_tag), path_(path), args_("RPC") {
    m_message.content.app_start_request.name = path_.data();
    m_message.content.app_start_request.args = args_.data();
}
AppExitRequest::AppExitRequest(uint32_t id) : MainRequest(id, PB_Main_app_exit_request_tag) {}
AppExchangeRequest::AppExchangeRequest(uint32_t id, const QByteArray &data)
    : MainRequest(id, PB_Main_app_data_exchange_request_tag),
      payload_(static_cast<pb_bytes_array_t *>(malloc(PB_BYTES_ARRAY_T_ALLOCSIZE(data.size()))), &free) {
    if (!payload_)
        throw std::bad_alloc();
    payload_->size = data.size();
    memcpy(payload_->bytes, data.constData(), data.size());
    m_message.content.app_data_exchange_request.data = payload_.get();
}
