#include "capture_store.h"
#include "file_crypto.h"
#include "operation_coordinator.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
using namespace UFlipper;
namespace fs = std::filesystem;
static void require(bool value, const char *why) {
    if (!value)
        throw std::runtime_error(why);
}
template <class F> static void rejects(F body, const char *why) {
    bool rejected = false;
    try {
        body();
    } catch (const std::exception &) {
        rejected = true;
    }
    require(rejected, why);
}
static std::string contents(const fs::path &p) {
    std::ifstream f(p, std::ios::binary);
    return {std::istreambuf_iterator<char>(f), {}};
}
static void ownership() {
    OperationCoordinator jobs;
    require(jobs.request("ir.receive", Control | Infrared, Origin::Ui).decision == Decision::Disconnected,
            "offline operation was allowed");
    jobs.connectDevice("test-device");
    auto first = jobs.request("ir.receive", Control | Infrared, Origin::Ui, true, false);
    require(first.decision == Decision::RecordingUnavailable, "live operation allowed without recording");
    first = jobs.request("ir.receive", Control | Infrared, Origin::Ui, true, true);
    require(first.decision == Decision::Granted, "first operation not granted");
    require(jobs.markRunning(first.id), "cannot mark running");
    auto next = jobs.request("nfc.scan", Control | Nfc, Origin::Mcp, true, true);
    require(next.decision == Decision::NeedsConfirmation, "MCP bypassed an active measurement");
    require(jobs.get(first.id).state == JobState::Running, "request silently stopped measurement");
    require(jobs.resolveConflict(next.id, true), "could not accept conflict");
    require(jobs.get(first.id).state == JobState::Stopping, "existing operation not stopping");
    require(jobs.get(next.id).state == JobState::Waiting, "replacement started before stop acknowledgement");
    require(jobs.acknowledgeStop(first.id), "stop acknowledgement failed");
    require(jobs.get(next.id).state == JobState::Starting, "replacement did not start after acknowledgement");
    require(jobs.markRunning(next.id), "cannot run replacement");
    jobs.disconnectDevice("USB disconnected");
    require(jobs.get(next.id).state == JobState::Unknown, "disconnect was treated as confirmed stop");
    jobs.connectDevice("other-device");
    require(!jobs.markRunning(next.id), "old generation operation became active");
    auto a = jobs.request("ir.receive", Control | Infrared, Origin::Ui, true, true);
    jobs.markRunning(a.id);
    auto b = jobs.request("subghz.receive", Control | Radio, Origin::Reader, true, true);
    jobs.resolveConflict(b.id, false);
    require(jobs.get(a.id).state == JobState::Running, "rejecting conflict interrupted operation");
    require(jobs.get(b.id).state == JobState::Canceled, "rejected replacement not canceled");
    jobs.requestStop(a.id);
    jobs.fail(a.id, "Stop timeout");
    auto timeout = jobs.request("nfc.scan", Control | Nfc, Origin::Mcp);
    require(timeout.decision == Decision::NeedsConfirmation, "unconfirmed stop released device resources");
    rejects([&] { jobs.request("bad", 0, Origin::Ui); }, "resource-free device operation accepted");
}
static void captures(const fs::path &root) {
    std::string id;
    {
        CaptureStore store(root);
        id = store.begin("ir.raw", "{\"firmware\":\"fixture\"}");
        std::vector<unsigned char> payload(64000, 0x55);
        for (int i = 0; i < 100; i++)
            store.append(id, payload.data(), payload.size(), i * 1000000ULL);
        store.finish(id, CaptureState::Completed, "User stop confirmed");
        require(store.verify(id), "completed capture failed verification");
        require(store.list().size() == 1, "capture not catalogued");
        require(store.list()[0].bytes > 6400000, "record headers or payload missing");
        require(contents(root / "recordings" / id / "manifest.json").find("completed") != std::string::npos,
                "manifest not finalized");
        std::ofstream corrupt(root / "recordings" / id / "events.bin", std::ios::binary | std::ios::app);
        corrupt.put('x');
        corrupt.close();
        require(!store.verify(id), "corruption went undetected");
    }
    {
        CaptureStore store(root);
        auto active = store.begin("uart", "fixture");
        const char bytes[] = "bytes";
        store.append(active, bytes, 5, 10);
        store.finish(active, CaptureState::Interrupted, "Test interruption");
        require(store.verify(active), "interrupted recording not readable");
        rejects([&] { store.append(active, bytes, 5, 20); }, "append after finalization accepted");
        auto huge = store.begin("ir.raw", "fixture");
        rejects([&] { store.append(huge, bytes, 2 * 1024 * 1024, 1); }, "oversize input not bounded");
        store.finish(huge, CaptureState::Interrupted, "Invalid frame");
    }
    auto unavailable = root / "not-a-directory";
    std::ofstream(unavailable) << "file";
    rejects([&] { CaptureStore store(unavailable); }, "unwritable recording root accepted");
}
static void crashRecovery(const fs::path &root) {
    auto pid = fork();
    require(pid >= 0, "Cannot start crash fixture");
    if (pid == 0) {
        try {
            CaptureStore store(root);
            auto id = store.begin("subghz.raw", "crash fixture");
            const char bytes[] = "pulse";
            store.append(id, bytes, 5, 1);
            _exit(0);
        } catch (...) {
            _exit(1);
        }
    }
    int status = 0;
    waitpid(pid, &status, 0);
    require(WIFEXITED(status) && WEXITSTATUS(status) == 0, "Crash fixture failed");
    CaptureStore reopened(root);
    for (const auto &item : reopened.list())
        require(item.state != CaptureState::Recording, "orphan recording remains live");
    auto items = reopened.list();
    require(items.size() == 1 && items[0].state == CaptureState::Interrupted, "interrupted capture missing");
    require(reopened.verify(items[0].id), "recovered capture checksum missing");
}
static void encryption(const fs::path &root) {
    fs::create_directories(root);
    auto key = FileCrypto::randomKey();
    auto other = FileCrypto::randomKey();
    auto plain = root / "plain.bin";
    {
        std::ofstream f(plain, std::ios::binary);
        for (int i = 0; i < 300000; i++)
            f.put(static_cast<char>(i % 251));
    }
    FileCrypto::encrypt(plain, root / "sealed.bin", key);
    require(contents(root / "sealed.bin") != contents(plain), "encryption produced cleartext");
    FileCrypto::decrypt(root / "sealed.bin", root / "recovered.bin", key);
    require(contents(plain) == contents(root / "recovered.bin"), "file did not round trip");
    rejects([&] { FileCrypto::decrypt(root / "sealed.bin", root / "wrong.bin", other); },
            "wrong key accepted");
    require(!fs::exists(root / "wrong.bin"), "failed decryption published plaintext");
    auto damaged = contents(root / "sealed.bin");
    damaged.pop_back();
    {
        std::ofstream f(root / "truncated.bin", std::ios::binary);
        f.write(damaged.data(), damaged.size());
    }
    rejects([&] { FileCrypto::decrypt(root / "truncated.bin", root / "partial.bin", key); },
            "truncated stream accepted");
    require(!fs::exists(root / "partial.bin"), "truncated plaintext published");
    auto salt = FileCrypto::randomSalt();
    auto a = FileCrypto::deriveKey("test passphrase", salt);
    auto b = FileCrypto::deriveKey("test passphrase", salt);
    require(a == b, "password key not deterministic with stored salt");
    require(FileCrypto::deriveKey("different password", salt) != a, "different password derived same key");
    require(FileCrypto::decodeKey(FileCrypto::encodeKey(key)) == key, "recovery code failed round trip");
    rejects([&] { FileCrypto::decodeKey("invalid"); }, "invalid recovery code accepted");
}
int main() {
    auto root = fs::temp_directory_path() / ("uflipper-core-tests-" + std::to_string(getpid()));
    fs::remove_all(root);
    fs::create_directories(root);
    try {
        ownership();
        captures(root / "captures");
        crashRecovery(root / "crash");
        encryption(root / "crypto");
        fs::remove_all(root);
        std::cout << "PASS: ownership, stop acknowledgement, recording integrity/recovery, authenticated "
                     "encryption\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "FAIL: " << e.what() << "\n";
        return 1;
    }
}
