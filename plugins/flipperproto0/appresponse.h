#pragma once
#include "appresponseinterface.h"
#include "mainresponse.h"
class AppStateResponse : public MainResponse, public AppStateResponseInterface {
    Q_OBJECT
    Q_INTERFACES(AppStateResponseInterface)
  public:
    using MainResponse::MainResponse;
    bool started() const override;
};
class AppDataResponse : public MainResponse, public AppDataResponseInterface {
    Q_OBJECT
    Q_INTERFACES(AppDataResponseInterface)
  public:
    using MainResponse::MainResponse;
    QByteArray data() const override;
};
