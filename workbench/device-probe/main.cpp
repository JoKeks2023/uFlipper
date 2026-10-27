#include "../../helpers/nfc_bridge/bridge_protocol.h"
#include "appresponseinterface.h"
#include "capture_store.h"
#include "deviceregistry.h"
#include "flipperzero/devicestate.h"
#include "flipperzero/flipperzero.h"
#include "flipperzero/protobufsession.h"
#include "flipperzero/rpc/appoperation.h"
#include "operation_coordinator.h"
#include "workbench/devicetelemetry.h"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QStandardPaths>
#include <QTimer>
#include <chrono>
#include <iostream>

// Development integration probe: no uploads, firmware actions or implicit helper installs.
class Probe : public QObject {
  public:
    Probe(bool helper, bool card, QObject *parent)
        : QObject(parent), telemetry_(&registry_, this), helper_(helper), card_(card) {
        watchdog_.setSingleShot(true);
        watchdog_.setInterval(30000);
        connect(&watchdog_, &QTimer::timeout, this, [this] { fail("Integration probe timed out"); });
        watchdog_.start();
        if (!helper_) {
            connect(&telemetry_, &DeviceTelemetry::snapshotChanged, this, [this] {
                auto snapshot = telemetry_.snapshot();
                if (snapshot.value("power").toMap().value("available").toBool()) {
                    std::cout
                        << QJsonDocument::fromVariant(snapshot).toJson(QJsonDocument::Compact).constData()
                        << '\n';
                    QCoreApplication::exit(0);
                }
            });
            telemetry_.setPollingEnabled(true);
        } else {
            poll_.setInterval(50);
            connect(&poll_, &QTimer::timeout, this, [this] {
                auto *current = registry_.currentDevice();
                if (current && current->deviceState()->isOnline() && current->rpc()->isSessionUp()) {
                    poll_.stop();
                    device_ = current;
                    beginHelper();
                }
            });
            poll_.start();
        }
    }

  private:
    void record(const QByteArray &bytes, UFlipper::RecordType type) {
        if (!store_ || recording_.empty())
            return;
        auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(
                       std::chrono::steady_clock::now().time_since_epoch())
                       .count();
        store_->append(recording_, bytes.constData(), bytes.size(), now, type);
    }
    void beginHelper() {
        jobs_.connectDevice(device_->deviceState()->deviceInfo().systemLocation.toStdString());
        if (card_) {
            auto path =
                QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/uFlipper/workbench";
            try {
                store_ = std::make_unique<UFlipper::CaptureStore>(path.toStdString());
                const auto &info = device_->deviceState()->deviceInfo();
                QJsonObject metadata{{"helper", "0.1.0"},
                                     {"purpose", "integration-test"},
                                     {"deviceName", info.name},
                                     {"firmwareVersion", info.firmware.version},
                                     {"firmwareCommit", info.firmware.commit},
                                     {"hardwareTarget", info.hardware.target},
                                     {"rpcMajor", int(info.protobuf.versionMajor)},
                                     {"rpcMinor", int(info.protobuf.versionMinor)}};
                recording_ = store_->begin(
                    "nfc.iso14443-4a", QJsonDocument(metadata).toJson(QJsonDocument::Compact).toStdString());
            } catch (const std::exception &error) {
                fail(error.what());
                return;
            }
        }
        auto request =
            jobs_.request("nfc.helper.probe", UFlipper::Control | UFlipper::DeviceApp | UFlipper::Nfc,
                          UFlipper::Origin::Ui, card_, bool(store_));
        if (request.decision != UFlipper::Decision::Granted) {
            fail("Device request was not granted");
            return;
        }
        job_ = request.id;
        connect(device_->rpc(), &Flipper::Zero::ProtobufSession::broadcastResponseReceived, this,
                [this](QObject *message) {
                    if (auto *state = qobject_cast<AppStateResponseInterface *>(message)) {
                        if (state->started()) {
                            started_ = true;
                            maybeReady();
                        } else {
                            closed_ = true;
                            maybeFinished();
                        }
                    } else if (auto *response = qobject_cast<AppDataResponseInterface *>(message)) {
                        try {
                            dataReceived(response->data());
                        } catch (const std::exception &error) {
                            fail(error.what());
                        }
                    }
                });
        auto *start = device_->rpc()->appStart("/ext/apps/Tools/uf_nfc_bridge.fap");
        connect(start, &AbstractOperation::finished, this, [this, start] {
            if (start->isError()) {
                fail(start->errorString());
                return;
            }
            startAck_ = true;
            maybeReady();
        });
    }
    void maybeReady() {
        if (started_ && startAck_ && !sentInfo_) {
            sentInfo_ = true;
            jobs_.markRunning(job_);
            send(UfNfcInfo);
        }
    }
    void send(uint8_t command, const QByteArray &payload = {}) {
        expectedCommand_ = command;
        ++sequence_;
        QByteArray frame(UF_NFC_HEADER, '\0');
        frame[0] = 'U';
        frame[1] = 'F';
        frame[2] = UF_NFC_VERSION;
        frame[3] = command;
        frame[4] = sequence_;
        frame[6] = char(payload.size());
        frame[7] = char(payload.size() >> 8);
        frame += payload;
        record(frame, UFlipper::RecordType::Status);
        auto *operation = device_->rpc()->appExchange(frame);
        connect(operation, &AbstractOperation::finished, this, [this, operation] {
            if (operation->isError())
                fail(operation->errorString());
        });
    }
    void dataReceived(const QByteArray &frame) {
        record(frame, UFlipper::RecordType::Data);
        auto *bytes = reinterpret_cast<const uint8_t *>(frame.constData());
        if (frame.size() < 8 || frame.size() > UF_NFC_HEADER + UF_NFC_MAX_RESPONSE || bytes[0] != 'U' ||
            bytes[1] != 'F' || bytes[2] != UF_NFC_VERSION || bytes[3] != (expectedCommand_ | 0x80) ||
            bytes[4] != sequence_ || uf_nfc_payload_size(bytes) != size_t(frame.size() - UF_NFC_HEADER))
            throw std::runtime_error("Invalid or mismatched NFC helper response");
        if (bytes[5] != UfNfcOk)
            throw std::runtime_error("NFC helper returned status " + std::to_string(bytes[5]));
        auto payload = frame.mid(8);
        if (expectedCommand_ == UfNfcInfo) {
            if (!payload.startsWith("uf-nfc/0.1.0;"))
                throw std::runtime_error("Unexpected helper version");
            if (card_)
                send(UfNfcActivate);
            else
                stopHelper();
        } else if (expectedCommand_ == UfNfcActivate) {
            if (payload.size() < 4 || uint8_t(payload[0]) > 10 || payload.size() != uint8_t(payload[0]) + 3)
                throw std::runtime_error("Invalid activated card metadata");
            send(UfNfcTransceive, QByteArray::fromHex("00a4040007d276000085010100"));
        } else if (expectedCommand_ == UfNfcTransceive) {
            if (payload.size() < 2 || payload.right(2) != QByteArray::fromHex("9000"))
                throw std::runtime_error("Test card APDU did not return 9000");
            if (apdus_ == 2 && payload.size() < 17)
                throw std::runtime_error("Test card capability container is truncated");
            ++apdus_;
            if (apdus_ == 1)
                send(UfNfcTransceive, QByteArray::fromHex("00a4000c02e103"));
            else if (apdus_ == 2)
                send(UfNfcTransceive, QByteArray::fromHex("00b000000f"));
            else
                send(UfNfcDeactivate);
        } else if (expectedCommand_ == UfNfcDeactivate)
            stopHelper();
    }
    void stopHelper() {
        if (stopping_ || !device_)
            return;
        stopping_ = true;
        jobs_.requestStop(job_);
        auto *stop = device_->rpc()->appExit();
        connect(stop, &AbstractOperation::finished, this, [this, stop] {
            if (stop->isError()) {
                failure_ = stop->errorString();
                finish(false);
            } else {
                stopAck_ = true;
                maybeFinished();
            }
        });
        QTimer::singleShot(4000, this, [this] {
            if (!finished_)
                finish(false);
        });
    }
    void maybeFinished() {
        if (stopping_ && stopAck_ && closed_)
            finish(true);
    }
    void fail(const QString &error) {
        if (finished_)
            return;
        failure_ = error;
        if (!helper_) {
            std::cerr << "Power query failed: " << error.toStdString() << '\n';
            finished_ = true;
            QCoreApplication::exit(1);
            return;
        }
        if (started_ && device_)
            stopHelper();
        else
            finish(false);
    }
    void finish(bool stopped) {
        if (finished_)
            return;
        finished_ = true;
        watchdog_.stop();
        bool success = stopped && failure_.isEmpty();
        if (!job_.empty()) {
            if (stopped)
                jobs_.acknowledgeStop(job_);
            else
                jobs_.fail(job_, "Helper stop not confirmed");
        }
        bool verified = false;
        try {
            if (store_ && !recording_.empty()) {
                store_->finish(recording_,
                               success ? UFlipper::CaptureState::Completed
                                       : UFlipper::CaptureState::Interrupted,
                               success ? "Helper and card session stopped" : failure_.toStdString());
                verified = store_->verify(recording_);
            }
        } catch (const std::exception &error) {
            success = false;
            failure_ = QString::fromUtf8(error.what());
        }
        QJsonObject result{{"helperVersion", "0.1.0"},
                           {"success", success},
                           {"stopConfirmed", stopped},
                           {"dependentApdus", apdus_},
                           {"error", failure_}};
        if (!recording_.empty()) {
            result.insert("recordingId", QString::fromStdString(recording_));
            result.insert("recordingVerified", verified);
            if (!verified)
                success = false;
        }
        result.insert("success", success);
        std::cout << QJsonDocument(result).toJson(QJsonDocument::Compact).constData() << '\n';
        QCoreApplication::exit(success ? 0 : 1);
    }
    Flipper::DeviceRegistry registry_;
    DeviceTelemetry telemetry_;
    QPointer<Flipper::FlipperZero> device_;
    QTimer poll_, watchdog_;
    UFlipper::OperationCoordinator jobs_;
    std::unique_ptr<UFlipper::CaptureStore> store_;
    std::string job_, recording_;
    QString failure_;
    uint8_t expectedCommand_ = 0, sequence_ = 0;
    int apdus_ = 0;
    bool helper_, card_, started_ = false, startAck_ = false, sentInfo_ = false, stopping_ = false,
                         stopAck_ = false, closed_ = false, finished_ = false;
};
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("uFlipper-probe");
    QCoreApplication::setOrganizationName("Joris Conrad");
    QCommandLineParser parser;
    parser.setApplicationDescription(
        "Development probe: power info or already approved NFC helper. No installation or firmware actions.");
    parser.addHelpOption();
    parser.addOption(
        QCommandLineOption(QStringLiteral("helper"),
                           QStringLiteral("Start installed uf_nfc_bridge 0.1.0 and verify Info/Exit")));
    parser.addOption(QCommandLineOption(
        QStringLiteral("card-test"),
        QStringLiteral("Read NDEF Type 4A test card with three dependent APDUs; requires --helper")));
    parser.addOption(QCommandLineOption(QStringLiteral("plugins"),
                                        QStringLiteral("Protobuf plugin directory"), QStringLiteral("path")));
    parser.process(app);
    if (parser.isSet("card-test") && !parser.isSet("helper"))
        parser.showHelp(2);
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/plugins");
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/../PlugIns");
    if (parser.isSet("plugins"))
        QCoreApplication::addLibraryPath(parser.value("plugins"));
    Probe probe(parser.isSet("helper"), parser.isSet("card-test"), &app);
    return app.exec();
}
