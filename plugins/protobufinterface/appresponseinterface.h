#pragma once
#include <QtPlugin>
#include <QByteArray>
class AppStateResponseInterface {
public:
    virtual ~AppStateResponseInterface() = default;
    virtual bool started() const = 0;
};
class AppDataResponseInterface {
public:
    virtual ~AppDataResponseInterface() = default;
    virtual QByteArray data() const = 0;
};
Q_DECLARE_INTERFACE(AppStateResponseInterface, "com.jorisconrad.uFlipper.AppStateResponse/1.0")
Q_DECLARE_INTERFACE(AppDataResponseInterface, "com.jorisconrad.uFlipper.AppDataResponse/1.0")
