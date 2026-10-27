#include "appresponseinterface.h"
#include "flipperzero/rpc/appoperation.h"
#include "mainresponseinterface.h"
#include "protobufplugininterface.h"
#include <QCoreApplication>
#include <QDebug>
#include <QPluginLoader>
#include <memory>
#include <stdexcept>
static void require(bool ok, const char *why) {
    if (!ok)
        throw std::runtime_error(why);
}
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    if (argc != 2)
        return 2;
    try {
        QPluginLoader loader(QString::fromLocal8Bit(argv[1]));
        auto *encoder = qobject_cast<ProtobufPluginInterface *>(loader.instance());
        if (!encoder)
            throw std::runtime_error(loader.errorString().toStdString());
        require(encoder->appStart(1, "/ext/apps/Tools/uf_nfc_bridge.fap").contains("RPC"),
                "start missing RPC context argument");
        require(encoder->appStart(1, QByteArray(513, 'a')).isEmpty(), "oversize app path accepted");
        require(encoder->appStart(1, QByteArray("a\0b", 3)).isEmpty(), "embedded NUL accepted in app path");
        require(encoder->appExchange(2, QByteArray(4097, 'a')).isEmpty(), "oversize payload accepted");
        require(!encoder->appExchange(2, QByteArray(4096, '\0')).isEmpty(),
                "bounded binary exchange could not encode");
        // Independent binary fixtures: length-delimited PB_Main, command ID 1.
        auto decode = [&](const char *hex) {
            return std::unique_ptr<QObject>(encoder->decode(QByteArray::fromHex(hex)));
        };
        auto state = decode("070801d203020801");
        auto *stateResponse = qobject_cast<AppStateResponseInterface *>(state.get());
        require(stateResponse && stateResponse->started(), "AppStarted broadcast not decoded");
        auto bytes = decode("0a08018a04050a0300ff42");
        auto *data = qobject_cast<AppDataResponseInterface *>(bytes.get());
        require(data && data->data() == QByteArray::fromHex("00ff42"), "binary AppData bytes changed");
        auto fail = decode("06080110162200");
        require(bool(fail), "error fixture could not decode");
        Flipper::Zero::AppOperation operation(1, Flipper::Zero::AppOperation::Start, "fixture");
        operation.feedResponse(fail.get());
        require(operation.isFinished() && operation.isError(), "app error reported successful");
        auto success = decode("0408012200");
        require(bool(success), "success fixture could not decode");
        Flipper::Zero::AppOperation good(1, Flipper::Zero::AppOperation::Exit, {});
        good.feedResponse(success.get());
        require(good.isFinished() && !good.isError(), "successful app exit failed");
        qInfo() << "PASS: App-RPC bounds, binary data, state broadcasts and command errors";
        return 0;
    } catch (const std::exception &error) {
        qCritical() << error.what();
        return 1;
    }
}
