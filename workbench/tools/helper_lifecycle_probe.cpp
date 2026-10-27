#include "bridge_protocol.h"
#include "capture_store.h"
#include "flipper.pb.h"
#include "io_utils.h"
#include "operation_coordinator.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "serial_console.h"
#include <chrono>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <thread>
#include <vector>
using namespace UFlipper;
using Clock = std::chrono::steady_clock;
struct Decoded {
    PB_Main message = PB_Main_init_zero;
    ~Decoded() {
        pb_release(&PB_Main_msg, &message);
    }
};
class Wire {
  public:
    Wire(SerialConsole &console, std::function<void(const std::string &, RecordType)> record)
        : console_(console), record_(std::move(record)) {}
    void setRecording(bool enabled) {
        recording_ = enabled;
    }
    void send(const PB_Main &message) {
        std::vector<unsigned char> bytes(8192);
        auto stream = pb_ostream_from_buffer(bytes.data(), bytes.size());
        if (!pb_encode_ex(&stream, &PB_Main_msg, &message, PB_ENCODE_DELIMITED))
            throw std::runtime_error("Cannot encode bounded RPC request");
        std::string encoded(reinterpret_cast<char *>(bytes.data()), stream.bytes_written);
        if (recording_)
            record_(encoded, RecordType::Status);
        console_.send(encoded);
    }
    std::unique_ptr<Decoded> next(Clock::time_point deadline) {
        while (Clock::now() < deadline) {
            uint32_t size = 0;
            size_t prefix = 0;
            bool complete = false;
            for (size_t i = 0; i < buffer_.size() && i < 5; ++i) {
                auto byte = static_cast<unsigned char>(buffer_[i]);
                if (i == 4 && (byte & 0xf0))
                    throw std::runtime_error("RPC size prefix overflow");
                size |= uint32_t(byte & 0x7f) << (i * 7);
                prefix = i + 1;
                if (!(byte & 0x80)) {
                    complete = true;
                    break;
                }
            }
            if (!complete && buffer_.size() >= 5)
                throw std::runtime_error("Invalid RPC size prefix");
            if (complete) {
                if (size > 8192)
                    throw std::runtime_error("RPC frame exceeds probe limit");
                if (buffer_.size() >= prefix + size) {
                    auto result = std::make_unique<Decoded>();
                    auto stream = pb_istream_from_buffer(reinterpret_cast<const pb_byte_t *>(buffer_.data()),
                                                         prefix + size);
                    if (!pb_decode_ex(&stream, &PB_Main_msg, &result->message, PB_DECODE_DELIMITED))
                        throw std::runtime_error("Cannot decode RPC response");
                    buffer_.erase(0, prefix + size);
                    return result;
                }
            }
            auto bytes = console_.read(std::chrono::milliseconds(50));
            if (recording_ && !bytes.empty())
                record_(bytes, RecordType::Data);
            buffer_ += bytes;
            if (buffer_.size() > 65536)
                throw std::runtime_error("RPC receive buffer limit reached");
        }
        throw std::runtime_error("RPC response timed out");
    }

  private:
    SerialConsole &console_;
    std::function<void(const std::string &, RecordType)> record_;
    std::string buffer_;
    bool recording_ = true;
};
static PB_Main request(uint32_t id, pb_size_t tag) {
    PB_Main message = PB_Main_init_zero;
    message.command_id = id;
    message.which_content = tag;
    return message;
}
int main(int argc, char **argv) {
    try {
        std::map<std::string, std::string> args;
        for (int i = 1; i < argc; i += 2) {
            if (i + 1 >= argc)
                throw std::invalid_argument("Expected --port PATH --store DIRECTORY");
            if (std::string(argv[i]) != "--port" && std::string(argv[i]) != "--store")
                throw std::invalid_argument("Unknown option");
            args[argv[i]] = argv[i + 1];
        }
        CaptureStore store(args.at("--store"));
        SerialConsole console(args.at("--port"));
        console.send("\r");
        if (!console.waitForPrompt(std::chrono::seconds(3)))
            throw std::runtime_error("Device CLI is not ready");
        std::string deviceInfo;
        console.send("device_info\r");
        if (!console.waitForPrompt(std::chrono::seconds(3), [&](const std::string &bytes) {
                if (deviceInfo.size() + bytes.size() > 65536)
                    throw std::runtime_error("Device information too large");
                deviceInfo += bytes;
            }))
            throw std::runtime_error("Device information timed out");
        auto id = store.begin(
            "nfc.helper.lifecycle",
            "{\"helper\":\"0.1.0\",\"radio_field\":false,\"device_info\":" + jsonQuote(deviceInfo) + "}");
        auto record = [&](const std::string &bytes, RecordType type) {
            auto ns =
                std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count();
            store.append(id, bytes.data(), bytes.size(), ns, type);
        };
        console.send("start_rpc_session\r");
        std::string echo;
        const std::string marker = "start_rpc_session\r\n";
        auto initDeadline = Clock::now() + std::chrono::seconds(3);
        while (Clock::now() < initDeadline && echo.find(marker) == std::string::npos) {
            echo += console.read(std::chrono::milliseconds(50));
            if (echo.size() > 65536)
                throw std::runtime_error("RPC startup response exceeds limit");
        }
        if (echo.find(marker) == std::string::npos)
            throw std::runtime_error("RPC startup not acknowledged");
        Wire wire(console, record);
        OperationCoordinator jobs;
        jobs.connectDevice(args.at("--port"));
        auto job = jobs.request("nfc.helper.lifecycle", Control | DeviceApp | Nfc, Origin::Ui, false, true);
        bool started = false, closed = false, acked = false, infoSeen = false, stopConfirmed = false,
             rpcClosed = false;
        std::string version;
        uint32_t activeId = 1;
        auto pump = [&](Clock::time_point deadline) {
            auto decoded = wire.next(deadline);
            const auto &message = decoded->message;
            if (message.command_id == activeId) {
                if (message.command_status != PB_CommandStatus_OK)
                    throw std::runtime_error("RPC command status " + std::to_string(message.command_status));
                if (message.which_content == PB_Main_empty_tag)
                    acked = true;
            }
            if (message.which_content == PB_Main_app_state_response_tag) {
                if (message.content.app_state_response.state == PB_App_AppState_APP_STARTED)
                    started = true;
                else
                    closed = true;
            }
            if (message.which_content == PB_Main_app_data_exchange_request_tag) {
                const auto *data = message.content.app_data_exchange_request.data;
                if (!data || data->size < 8 || data->size > UF_NFC_HEADER + UF_NFC_MAX_RESPONSE)
                    throw std::runtime_error("Invalid helper response size");
                const auto *b = data->bytes;
                if (b[0] != 'U' || b[1] != 'F' || b[2] != UF_NFC_VERSION || b[3] != (UfNfcInfo | 0x80) ||
                    b[4] != 1 || b[5] != 0 || uf_nfc_payload_size(b) != data->size - 8)
                    throw std::runtime_error("Mismatched helper response");
                version.assign(reinterpret_cast<const char *>(b + 8), data->size - 8);
                if (version.rfind("uf-nfc/0.1.0;", 0) != 0)
                    throw std::runtime_error("Unexpected helper version");
                infoSeen = true;
            }
        };
        try {
            auto start = request(1, PB_Main_app_start_request_tag);
            char path[] = "/ext/apps/Tools/uf_nfc_bridge.fap", mode[] = "RPC";
            start.content.app_start_request.name = path;
            start.content.app_start_request.args = mode;
            wire.send(start);
            auto deadline = Clock::now() + std::chrono::seconds(5);
            while (!started || !acked)
                pump(deadline);
            jobs.markRunning(job.id);
            activeId = 2;
            acked = false;
            auto info = request(2, PB_Main_app_data_exchange_request_tag);
            struct {
                pb_size_t size;
                pb_byte_t bytes[8];
            } payload = {8, {'U', 'F', 1, UfNfcInfo, 1, 0, 0, 0}};
            info.content.app_data_exchange_request.data = reinterpret_cast<pb_bytes_array_t *>(&payload);
            wire.send(info);
            deadline = Clock::now() + std::chrono::seconds(4);
            while (!acked || !infoSeen)
                pump(deadline);
            activeId = 3;
            acked = false;
            jobs.requestStop(job.id);
            wire.send(request(3, PB_Main_app_exit_request_tag));
            deadline = Clock::now() + std::chrono::seconds(4);
            while (!acked || !closed)
                pump(deadline);
            stopConfirmed = true;
            jobs.acknowledgeStop(job.id);
            activeId = 4;
            acked = false;
            wire.send(request(4, PB_Main_stop_session_tag));
            deadline = Clock::now() + std::chrono::seconds(3);
            while (!acked)
                pump(deadline);
            // Momentum's CLI transport may be blocked waiting for its next input
            // byte after the RPC worker acknowledges closure. Wake that read.
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            console.send("\r");
            rpcClosed = console.waitForPrompt(
                std::chrono::seconds(3), [&](const std::string &bytes) { record(bytes, RecordType::Data); });
            store.finish(id, rpcClosed ? CaptureState::Completed : CaptureState::Interrupted,
                         rpcClosed ? "Helper start/version/stop and RPC closure confirmed"
                                   : "Helper stopped; RPC closure not confirmed");
        } catch (...) {
            wire.setRecording(false);
            try {
                activeId = 99;
                acked = false;
                wire.send(request(99, PB_Main_app_exit_request_tag));
                auto deadline = Clock::now() + std::chrono::seconds(2);
                while (!acked || !closed)
                    pump(deadline);
            } catch (...) {
            }
            try {
                wire.send(request(100, PB_Main_stop_session_tag));
                console.waitForPrompt(std::chrono::seconds(2));
            } catch (...) {
            }
            try {
                store.finish(id, CaptureState::Interrupted, "Helper lifecycle probe failed");
            } catch (...) {
            }
            throw;
        }
        std::cout << "{\"recording_id\":" << jsonQuote(id) << ",\"helper\":" << jsonQuote(version)
                  << ",\"started\":" << (started ? "true" : "false")
                  << ",\"info_verified\":" << (infoSeen ? "true" : "false")
                  << ",\"stop_confirmed\":" << (stopConfirmed ? "true" : "false")
                  << ",\"rpc_closed\":" << (rpcClosed ? "true" : "false")
                  << ",\"recording_verified\":" << (store.verify(id) ? "true" : "false")
                  << ",\"card_activated\":false}\n";
        return stopConfirmed && rpcClosed && store.verify(id) ? 0 : 2;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
