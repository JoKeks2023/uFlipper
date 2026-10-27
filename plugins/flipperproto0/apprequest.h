#pragma once
#include "mainrequest.h"
#include <cstdlib>
#include <memory>
class AppStartRequest : public MainRequest {
  public:
    AppStartRequest(uint32_t id, const QByteArray &path);

  private:
    QByteArray path_, args_;
};
class AppExitRequest : public MainRequest {
  public:
    explicit AppExitRequest(uint32_t id);
};
class AppExchangeRequest : public MainRequest {
  public:
    AppExchangeRequest(uint32_t id, const QByteArray &data);

  private:
    std::unique_ptr<pb_bytes_array_t, decltype(&free)> payload_;
};
