#include "capture_store.h"
#include "io_utils.h"
#include "operation_coordinator.h"
#include "receive_status.h"
#include "serial_console.h"
#include <chrono>
#include <iostream>
#include <map>
#include <stdexcept>
using namespace UFlipper;
int main(int argc, char **argv) {
    try {
        std::map<std::string, std::string> args;
        for (int i = 1; i < argc; i += 2) {
            if (i + 1 >= argc)
                throw std::invalid_argument(
                    "Expected --port PATH --kind ir.raw|ir.decoded|subghz.raw|subghz.decoded --seconds N "
                    "--store PATH [--frequency HZ]");
            args[argv[i]] = argv[i + 1];
        }
        for (const auto &pair : args)
            if (pair.first != "--port" && pair.first != "--kind" && pair.first != "--seconds" &&
                pair.first != "--store" && pair.first != "--frequency")
                throw std::invalid_argument("Unknown option");
        auto kind = args.at("--kind");
        bool radio = kind == "subghz.raw" || kind == "subghz.decoded";
        if (!radio && kind != "ir.raw" && kind != "ir.decoded")
            throw std::invalid_argument("Unsupported receive kind");
        size_t used = 0;
        int seconds = std::stoi(args.at("--seconds"), &used);
        if (used != args.at("--seconds").size() || seconds < 1 || seconds > 300)
            throw std::invalid_argument("Seconds must be 1..300");
        std::string command = kind == "ir.raw" ? "ir rx raw" : "ir rx";
        if (radio) {
            auto value = args.count("--frequency") ? args.at("--frequency") : "433920000";
            used = 0;
            auto hz = std::stoul(value, &used);
            if (used != value.size() ||
                !((hz >= 300000000 && hz <= 348000000) || (hz >= 387000000 && hz <= 464000000) ||
                  (hz >= 779000000 && hz <= 928000000)))
                throw std::invalid_argument("Unsupported receive frequency");
            command = kind == "subghz.raw" ? "subghz rx_raw " : "subghz rx ";
            command += std::to_string(hz);
            if (kind == "subghz.decoded")
                command += " 0"; // Momentum requires the internal radio index.
        }
        CaptureStore store(args.at("--store"));
        SerialConsole console(args.at("--port"));
        console.send("\r");
        if (!console.waitForPrompt(std::chrono::seconds(3)))
            throw std::runtime_error("No CLI prompt; RPC or another operation may be active");
        std::string deviceInfo;
        console.send("device_info\r");
        if (!console.waitForPrompt(std::chrono::seconds(3), [&](const std::string &data) {
                if (deviceInfo.size() + data.size() <= 65536)
                    deviceInfo += data;
            }))
            throw std::runtime_error("Cannot read device information");
        auto id = store.begin(kind, "{\"command\":" + jsonQuote(command) +
                                        ",\"device_info\":" + jsonQuote(deviceInfo) + "}");
        OperationCoordinator jobs;
        jobs.connectDevice(args.at("--port"));
        auto job = jobs.request(kind, Control | (radio ? Radio : Infrared), Origin::Ui, true, true);
        uint64_t total = 0;
        ReceiveStatus status(kind);
        bool stopped = false;
        auto append = [&](const std::string &bytes) {
            auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(
                           std::chrono::steady_clock::now().time_since_epoch())
                           .count();
            store.append(id, bytes.data(), bytes.size(), now);
            total += bytes.size();
            status.feed(bytes);
        };
        try {
            console.send(command + "\r");
            auto startDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
            while (!status.started() && !status.returnedToPrompt() &&
                   std::chrono::steady_clock::now() < startDeadline) {
                auto data = console.read(std::chrono::milliseconds(100));
                if (!data.empty())
                    append(data);
            }
            if (status.returnedToPrompt() || !status.started())
                throw std::runtime_error(
                    "Firmware did not acknowledge receiver startup; inspect interrupted recording");
            jobs.markRunning(job.id);
            auto until = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
            while (std::chrono::steady_clock::now() < until) {
                auto data = console.read(std::chrono::milliseconds(100));
                if (!data.empty())
                    append(data);
                if (status.returnedToPrompt())
                    throw std::runtime_error("Receiver returned unexpectedly to CLI");
            }
            jobs.requestStop(job.id);
            stopped = console.stop(std::chrono::seconds(3), append);
            if (stopped)
                jobs.acknowledgeStop(job.id);
            else
                jobs.fail(job.id, "Stop was not acknowledged");
            store.finish(id, stopped ? CaptureState::Completed : CaptureState::Interrupted,
                         stopped ? "CLI prompt confirmed after Ctrl+C" : "No stop acknowledgement");
        } catch (...) {
            try {
                console.stop(std::chrono::seconds(2), append);
            } catch (...) {
            }
            try {
                store.finish(id, CaptureState::Interrupted, "Receive probe failed; inspect device state");
            } catch (...) {
            }
            throw;
        }
        std::cout << "{\"recording_id\":" << jsonQuote(id) << ",\"kind\":" << jsonQuote(kind)
                  << ",\"received_bytes\":" << total << ",\"ir_frames\":" << status.irFrames()
                  << ",\"start_confirmed\":" << (status.started() ? "true" : "false")
                  << ",\"stop_confirmed\":" << (stopped ? "true" : "false")
                  << ",\"recording_verified\":" << (store.verify(id) ? "true" : "false") << "}\n";
        return stopped ? 0 : 2;
    } catch (const std::exception &error) {
        std::cerr << error.what() << "\n";
        return 1;
    }
}
