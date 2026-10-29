#include "bridge_protocol.h"
#include "capture_store.h"
#include "flipper.pb.h"
#include "io_utils.h"
#include "operation_coordinator.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "rpc_probe_transport.h"
#include "serial_console.h"
#include <chrono>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <thread>
#include <vector>
using namespace UFlipper;
using namespace UFlipper::Probe;
int main(int argc, char **argv) {
    try {
        std::map<std::string, std::string> args;
        bool applePay = false;
        for (int i = 1; i < argc;) {
            if (std::string(argv[i]) == "--apple-pay") {
                applePay = true;
                ++i;
                continue;
            }
            if (i + 1 >= argc)
                throw std::invalid_argument("Expected --port PATH --store DIRECTORY [--apple-pay]");
            if (std::string(argv[i]) != "--port" && std::string(argv[i]) != "--store")
                throw std::invalid_argument("Unknown option");
            args[argv[i]] = argv[i + 1];
            i += 2;
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
            applePay ? "nfc.emv.ppse-probe" : "nfc.helper.lifecycle",
            "{\"helper\":\"0.1.0\",\"radio_field\":" + std::string(applePay ? "true" : "false") +
                ",\"apdu_policy\":" + jsonQuote(applePay ? "SELECT PPSE twice only; no payment processing" : "none") +
                ",\"device_info\":" +
                jsonQuote(deviceInfo) + "}");
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
        auto job = jobs.request(applePay ? "nfc.emv.ppse-probe" : "nfc.helper.lifecycle",
                                Control | DeviceApp | Nfc, Origin::Ui, applePay, true);
        bool started = false, closed = false, acked = false, infoSeen = false, stopConfirmed = false,
             rpcClosed = false, responseSeen = false, cardActivated = false;
        std::string version, helperResponse, failure;
        uint8_t expectedOpcode = UfNfcInfo, expectedSequence = 0, helperStatus = UfNfcInvalid;
        unsigned apduRoundTrips = 0;
        std::vector<std::pair<std::string, size_t>> ppseResults;
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
                if (b[0] != 'U' || b[1] != 'F' || b[2] != UF_NFC_VERSION || b[3] != (expectedOpcode | 0x80) ||
                    b[4] != expectedSequence || uf_nfc_payload_size(b) != data->size - 8)
                    throw std::runtime_error("Mismatched helper response");
                helperStatus = b[5];
                helperResponse.assign(reinterpret_cast<const char *>(b + 8), data->size - 8);
                responseSeen = true;
                if (expectedOpcode == UfNfcInfo && helperStatus == UfNfcOk) {
                    version = helperResponse;
                    if (version != "uf-nfc/0.1.0;iso14443-4a;short-apdu;max=240")
                        throw std::runtime_error("Unexpected helper version or capabilities");
                    infoSeen = true;
                }
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
            auto exchange = [&](uint8_t opcode, const std::string &bytes = std::string()) {
                expectedOpcode = opcode;
                ++expectedSequence;
                ++activeId;
                acked = false;
                responseSeen = false;
                helperResponse.clear();
                auto command = request(activeId, PB_Main_app_data_exchange_request_tag);
                struct {
                    pb_size_t size;
                    pb_byte_t bytes[UF_NFC_HEADER + UF_NFC_MAX_APDU];
                } payload{};
                if (bytes.size() > UF_NFC_MAX_APDU)
                    throw std::runtime_error("APDU exceeds the probe limit");
                payload.size = UF_NFC_HEADER + bytes.size();
                payload.bytes[0] = 'U';
                payload.bytes[1] = 'F';
                payload.bytes[2] = UF_NFC_VERSION;
                payload.bytes[3] = opcode;
                payload.bytes[4] = expectedSequence;
                payload.bytes[6] = bytes.size() & 0xff;
                payload.bytes[7] = bytes.size() >> 8;
                std::memcpy(payload.bytes + UF_NFC_HEADER, bytes.data(), bytes.size());
                if (!uf_nfc_valid_request(payload.bytes, payload.size))
                    throw std::runtime_error("Invalid bounded helper request");
                command.content.app_data_exchange_request.data = reinterpret_cast<pb_bytes_array_t *>(&payload);
                wire.send(command);
                auto exchangeDeadline = Clock::now() + std::chrono::seconds(5);
                while (!acked || !responseSeen)
                    pump(exchangeDeadline);
            };
            exchange(UfNfcInfo);
            if (!infoSeen)
                throw std::runtime_error("Helper version was not confirmed");
            if (applePay) {
                std::cerr << "NFC field starting: hold the authorised iPhone at the Flipper back.\n";
                auto activationDeadline = Clock::now() + std::chrono::seconds(24);
                do {
                    exchange(UfNfcActivate);
                    if (helperStatus == UfNfcOk)
                        break;
                    if (helperStatus != UfNfcNoCard)
                        throw std::runtime_error("Card activation returned helper status " +
                                                 std::to_string(helperStatus));
                } while (Clock::now() < activationDeadline);
                if (helperStatus != UfNfcOk)
                    throw std::runtime_error("No compatible ISO14443-4A card detected within 24 seconds");
                if (helperResponse.size() < 4 || static_cast<unsigned char>(helperResponse[0]) > 10 ||
                    helperResponse.size() != static_cast<unsigned char>(helperResponse[0]) + 3U)
                    throw std::runtime_error("Invalid card activation metadata");
                cardActivated = true;
                // Fixed directory-selection command only. No arbitrary APDUs,
                // account-record reads, GPO, GENERATE AC or payment transaction.
                const unsigned char ppse[] = {0x00, 0xa4, 0x04, 0x00, 0x0e, 0x32, 0x50, 0x41,
                                              0x59, 0x2e, 0x53, 0x59, 0x53, 0x2e, 0x44, 0x44,
                                              0x46, 0x30, 0x31, 0x00};
                for (int round = 0; round < 2; ++round) {
                    exchange(UfNfcTransceive,
                             std::string(reinterpret_cast<const char *>(ppse), sizeof ppse));
                    if (helperStatus != UfNfcOk)
                        throw std::runtime_error("APDU transport returned helper status " +
                                                 std::to_string(helperStatus));
                    if (helperResponse.size() < 2)
                        throw std::runtime_error("APDU response lacks a status word");
                    ++apduRoundTrips;
                    std::ostringstream status;
                    status << std::hex << std::setfill('0') << std::setw(2)
                           << unsigned(static_cast<unsigned char>(helperResponse[helperResponse.size() - 2]))
                           << std::setw(2) << unsigned(static_cast<unsigned char>(helperResponse.back()));
                    ppseResults.emplace_back(status.str(), helperResponse.size());
                    if (status.str() != "9000")
                        throw std::runtime_error("PPSE selection returned status " + status.str());
                }
                exchange(UfNfcDeactivate);
                if (helperStatus != UfNfcOk)
                    throw std::runtime_error("NFC deactivation was not confirmed");
            }
            ++activeId;
            acked = false;
            jobs.requestStop(job.id);
            wire.send(request(activeId, PB_Main_app_exit_request_tag));
            deadline = Clock::now() + std::chrono::seconds(4);
            while (!acked || !closed)
                pump(deadline);
            stopConfirmed = true;
            jobs.acknowledgeStop(job.id);
            ++activeId;
            acked = false;
            wire.send(request(activeId, PB_Main_stop_session_tag));
            deadline = Clock::now() + std::chrono::seconds(3);
            while (!acked)
                pump(deadline);
            // Momentum's CLI transport may be blocked waiting for its next input
            // byte after the RPC worker acknowledges closure. Wake that read.
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            console.send("\r");
            rpcClosed = console.waitForPrompt(
                std::chrono::seconds(3), [&](const std::string &bytes) { record(bytes, RecordType::Data); });
            const auto completion = applePay
                ? "PPSE directory selection twice; helper stop and RPC closure confirmed"
                : "Helper start/version/stop and RPC closure confirmed";
            store.finish(id, rpcClosed ? CaptureState::Completed : CaptureState::Interrupted,
                         rpcClosed ? completion : "Helper stopped; RPC closure not confirmed");
        } catch (const std::exception &error) {
            failure = error.what();
            wire.setRecording(false);
            try {
                record("Failure cleanup transport not recorded", RecordType::Gap);
            } catch (...) {
            }
            try {
                jobs.requestStop(job.id);
                activeId = 99;
                acked = false;
                wire.send(request(99, PB_Main_app_exit_request_tag));
                auto deadline = Clock::now() + std::chrono::seconds(2);
                while (!acked || !closed)
                    pump(deadline);
                stopConfirmed = true;
                jobs.acknowledgeStop(job.id);
            } catch (...) {
            }
            try {
                activeId = 100;
                acked = false;
                wire.send(request(100, PB_Main_stop_session_tag));
                auto deadline = Clock::now() + std::chrono::seconds(3);
                while (!acked)
                    pump(deadline);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                console.send("\r");
                rpcClosed = console.waitForPrompt(std::chrono::seconds(3));
            } catch (...) {
            }
            try {
                store.finish(id, CaptureState::Interrupted, failure);
            } catch (...) {
            }
        }
        std::cout << "{\"recording_id\":" << jsonQuote(id) << ",\"helper\":" << jsonQuote(version)
                  << ",\"started\":" << (started ? "true" : "false")
                  << ",\"info_verified\":" << (infoSeen ? "true" : "false")
                  << ",\"stop_confirmed\":" << (stopConfirmed ? "true" : "false")
                  << ",\"rpc_closed\":" << (rpcClosed ? "true" : "false")
                  << ",\"recording_verified\":" << (store.verify(id) ? "true" : "false")
                  << ",\"card_activated\":" << (cardActivated ? "true" : "false")
                  << ",\"apdu_round_trips\":" << apduRoundTrips
                  << ",\"error\":" << jsonQuote(failure)
                  << ",\"dependent_apdus_proven\":false,\"pcsc_verified\":false,\"ppse_results\":[";
        for (size_t i = 0; i < ppseResults.size(); ++i) {
            if (i)
                std::cout << ',';
            std::cout << "{\"status_word\":" << jsonQuote(ppseResults[i].first)
                      << ",\"response_bytes\":" << ppseResults[i].second << '}';
        }
        std::cout << "]}\n";
        return failure.empty() && stopConfirmed && rpcClosed && store.verify(id) ? 0 : 2;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
