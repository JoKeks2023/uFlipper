#include "appresponseinterface.h"
#include "applicationbackend.h"
#include "logger.h"
#include "flipperzero/rpc/storagereadoperation.h"
#include <QBuffer>
#include <cmath>
#include <QThread>
#include <thread>
#include "flipperzero/rpc/appoperation.h"
#include "mainresponseinterface.h"
#include "protobufplugininterface.h"
#include "workbench/powersnapshot.h"
#include "workbench/nfccontroller.h"
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
        auto *logger = Logger::instance();
        const auto before = logger->errorCount();
        bool wrongThread = false;
        auto listener = QObject::connect(logger, &Logger::errorCountChanged, logger, [&] {
            wrongThread |= QThread::currentThread() != logger->thread();
        }, Qt::DirectConnection);
        std::thread worker([] {
            Logger::messageOutput(QtCriticalMsg, QMessageLogContext(nullptr, 0, nullptr, "test.worker"),
                                  "worker log fixture");
        });
        worker.join();
        QCoreApplication::processEvents();
        QObject::disconnect(listener);
        require(!wrongThread && logger->errorCount() == before + 1 &&
                    logger->logText().contains("worker log fixture"),
                "worker logging touched UI state or dropped the queued message");
        // The actual Qt telemetry capture uses "f7", not the SDK's numeric 7.
        require(NfcController::approvedHelperTarget("e1784e74", "f7"), "Recorded target f7 was incorrectly rejected");
        require(!NfcController::approvedHelperTarget("e1784e74", "7") &&
                !NfcController::approvedHelperTarget("other-firmware", "f7"),
                "Unapproved firmware/target accepted for helper");
        ApplicationBackend guarded;
        int blocked=0;
        QObject::connect(&guarded,&ApplicationBackend::operationBlocked,[&](const QString& why) {
            require(!why.isEmpty(), "backup preflight did not explain the block"); ++blocked;
        });
        guarded.mainAction();
        guarded.restoreBackup(QUrl::fromLocalFile("/nonexistent/fixture.tar.gz"));
        guarded.installFirmware(QUrl::fromLocalFile("/nonexistent/fixture.dfu"));
        require(blocked==3 && guarded.backendState()==ApplicationBackend::BackendState::WaitingForDevices,
                "firmware or restore bypassed full backup preflight");
        auto missing = UFlipper::powerSnapshot(
            {{"format.major", "2"}, {"battery.current", "-128"}, {"charge.level", "999"}});
        auto metrics = missing.value("metrics").toMap();
        require(metrics.value("battery.current").toMap().value("value").toDouble() == -128,
                "signed current lost");
        require(!metrics.value("battery.voltage").toMap().value("available").toBool(),
                "missing voltage became zero");
        require(!metrics.value("charge.level").toMap().value("available").toBool(),
                "invalid battery level accepted");
        auto future = UFlipper::powerSnapshot({{"format.major", "3"}, {"charge.level", "50"}});
        require(!future.value("formatSupported").toBool(), "unknown power format interpreted");
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
        QBuffer emptyFile;
        Flipper::Zero::StorageReadOperation emptyRead(1,"/ext/empty",&emptyFile);
        emptyRead.start();emptyRead.encodeRequest(encoder);
        auto emptyStat=decode("070801ca01020a00");
        require(bool(emptyStat),"Empty stat fixture did not decode");emptyRead.feedResponse(emptyStat.get());emptyRead.encodeRequest(encoder);
        auto emptyChunk=decode("06080152020a00");
        require(bool(emptyChunk),"Empty read fixture did not decode");emptyRead.feedResponse(emptyChunk.get());
        require(emptyRead.isFinished() && !emptyRead.isError() && std::isfinite(emptyRead.progress()) && emptyRead.progress()==100 && emptyFile.data().isEmpty(),
                "Zero-byte file read produced invalid progress or incomplete output");
        qInfo() << "PASS: zero-byte storage read; App-RPC bounds, binary data, state broadcasts and command errors";
        return 0;
    } catch (const std::exception &error) {
        qCritical() << error.what();
        return 1;
    }
}
