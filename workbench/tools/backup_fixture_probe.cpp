#include "backup_store.h"
#include "io_utils.h"
#include "rpc_probe_transport.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <thread>
using namespace UFlipper;
using namespace UFlipper::Probe;

// Deliberately limited to this invocation's random fixture directory. This is
// an integration proof, not the full-device backup/restore implementation.
class FixtureStorage {
  public:
    FixtureStorage(Wire &wire, std::string root) : wire_(wire), root_(std::move(root)) {}
    void create() {
        mkdir("/ext/uflipper-tests", true);
        mkdir(root_, false); // Never reuse an existing test directory.
    }
    void mkdir(const std::string &path, bool allowExisting = false) {
        if (path != "/ext/uflipper-tests")
            checkPath(path);
        auto message = request(++sequence_, PB_Main_storage_mkdir_request_tag);
        message.content.storage_mkdir_request.path = const_cast<char *>(path.c_str());
        wire_.send(message);
        emptyReply(allowExisting);
    }
    void write(const std::string &path, const std::string &data) {
        checkPath(path);
        if (data.size() > 4096)
            throw std::runtime_error("Fixture exceeds transfer limit");
        auto message = request(++sequence_, PB_Main_storage_write_request_tag);
        message.content.storage_write_request.path = const_cast<char *>(path.c_str());
        message.content.storage_write_request.has_file = !data.empty();
        std::vector<pb_byte_t> memory(PB_BYTES_ARRAY_T_ALLOCSIZE(data.size()));
        auto *payload = reinterpret_cast<pb_bytes_array_t *>(memory.data());
        payload->size = data.size();
        std::copy(data.begin(), data.end(), payload->bytes);
        message.content.storage_write_request.file.data = payload;
        wire_.send(message);
        emptyReply();
    }
    std::string read(const std::string &path) {
        checkPath(path);
        auto message = request(++sequence_, PB_Main_storage_read_request_tag);
        message.content.storage_read_request.path = const_cast<char *>(path.c_str());
        wire_.send(message);
        std::string result;
        auto deadline = Clock::now() + std::chrono::seconds(5);
        bool more;
        do {
            auto decoded = reply(deadline);
            const auto &m = decoded->message;
            if (m.which_content != PB_Main_storage_read_response_tag ||
                !m.content.storage_read_response.has_file)
                throw std::runtime_error("Unexpected storage read response");
            auto *data = m.content.storage_read_response.file.data;
            if (data) {
                if (result.size() + data->size > 4096)
                    throw std::runtime_error("Fixture read exceeds transfer limit");
                result.append(reinterpret_cast<const char *>(data->bytes), data->size);
            }
            more = m.has_next;
        } while (more);
        return result;
    }
    void removeFixtures() {
        checkPath(root_);
        auto message = request(++sequence_, PB_Main_storage_delete_request_tag);
        message.content.storage_delete_request.path = const_cast<char *>(root_.c_str());
        message.content.storage_delete_request.recursive = true;
        wire_.send(message);
        emptyReply();
        message = request(++sequence_, PB_Main_storage_stat_request_tag);
        message.content.storage_stat_request.path = const_cast<char *>(root_.c_str());
        wire_.send(message);
        auto decoded = correlated(Clock::now() + std::chrono::seconds(5));
        if (decoded->message.command_status != PB_CommandStatus_ERROR_STORAGE_NOT_EXIST)
            throw std::runtime_error("Fixture directory removal not confirmed");
    }
    bool closeRpc(SerialConsole &console) {
        wire_.send(request(++sequence_, PB_Main_stop_session_tag));
        emptyReply();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        console.send("\r");
        return console.waitForPrompt(std::chrono::seconds(3));
    }

  private:
    void checkPath(const std::string &path) const {
        if (path != root_ && path.rfind(root_ + "/", 0) != 0)
            throw std::runtime_error("Write outside generated fixture directory denied");
        if (path.find("..") != std::string::npos || path.find('\0') != std::string::npos)
            throw std::runtime_error("Invalid fixture path");
    }
    std::unique_ptr<Decoded> correlated(Clock::time_point deadline) {
        while (Clock::now() < deadline) {
            auto message = wire_.next(deadline);
            if (message->message.command_id == sequence_)
                return message;
            if (message->message.command_id != 0)
                throw std::runtime_error("Unexpected RPC response correlation");
        }
        throw std::runtime_error("Storage response timed out");
    }
    std::unique_ptr<Decoded> reply(Clock::time_point deadline) {
        auto result = correlated(deadline);
        if (result->message.command_status != PB_CommandStatus_OK)
            throw std::runtime_error("Storage RPC status " + std::to_string(result->message.command_status));
        return result;
    }
    void emptyReply(bool allowExisting = false) {
        auto result = correlated(Clock::now() + std::chrono::seconds(5));
        const auto &m = result->message;
        if (allowExisting && m.command_status == PB_CommandStatus_ERROR_STORAGE_EXIST)
            return;
        if (m.command_status != PB_CommandStatus_OK || m.which_content != PB_Main_empty_tag || m.has_next)
            throw std::runtime_error("Storage operation not confirmed: " + std::to_string(m.command_status));
    }
    Wire &wire_;
    std::string root_;
    uint32_t sequence_ = 0;
};

static std::string openRpc(SerialConsole &console) {
    console.send("\r");
    if (!console.waitForPrompt(std::chrono::seconds(3)))
        throw std::runtime_error("Device CLI is not ready");
    std::string info;
    console.send("device_info\r");
    if (!console.waitForPrompt(std::chrono::seconds(3), [&](const std::string &bytes) {
            if (info.size() + bytes.size() > 65536)
                throw std::runtime_error("Device information exceeds limit");
            info += bytes;
        }))
        throw std::runtime_error("Device information timed out");
    console.send("start_rpc_session\r");
    std::string echo;
    auto deadline = Clock::now() + std::chrono::seconds(3);
    while (Clock::now() < deadline) {
        echo += console.read(std::chrono::milliseconds(50));
        if (echo.find("start_rpc_session\r\n") != std::string::npos)
            return info;
        if (echo.size() > 65536)
            throw std::runtime_error("RPC startup exceeds limit");
    }
    throw std::runtime_error("RPC startup not acknowledged");
}

int main(int argc, char **argv) {
    std::string recordingId, root;
    try {
        std::map<std::string, std::string> args;
        for (int i = 1; i < argc; i += 2) {
            if (i + 1 >= argc || (std::string(argv[i]) != "--port" && std::string(argv[i]) != "--store"))
                throw std::invalid_argument("Expected --port PATH --store DIRECTORY");
            args[argv[i]] = argv[i + 1];
        }
        CaptureStore captures(args.at("--store"));
        BackupStore backups(args.at("--store"));
        root = "/ext/uflipper-tests/" + randomId();
        // Local writable storage is established before the first device mutation.
        recordingId = captures.begin("backup.fixture.roundtrip",
                                     "{\"scope\":\"test-subset\",\"root\":" + jsonQuote(root) + "}");
        SerialConsole console(args.at("--port"));
        auto deviceInfo = openRpc(console);
        auto record = [&](const std::string &bytes, RecordType type) {
            auto ns =
                std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count();
            captures.append(recordingId, bytes.data(), bytes.size(), ns, type);
        };
        record(deviceInfo, RecordType::Status);
        Wire wire(console, record);
        FixtureStorage device(wire, root);
        try {
            device.create();
            device.mkdir(root + "/empty");
            std::map<std::string, std::string> original;
            original[root + "/notes.txt"] = "uFlipper isolated restore fixture\n" + root + "\n";
            auto &binary = original[root + "/bytes.bin"];
            for (int i = 0; i < 3072; ++i)
                binary.push_back(static_cast<char>((i * 37) % 256));
            for (const auto &[path, bytes] : original)
                device.write(path, bytes);
            auto backup = [&](const std::map<std::string, std::string> &expected,
                              const std::string &purpose) {
                auto id = backups.begin(BackupScope::TestSubset,
                                        "{\"purpose\":" + jsonQuote(purpose) +
                                            ",\"device_info\":" + jsonQuote(deviceInfo) + "}");
                try {
                    backups.addDirectory(id, root);
                    backups.addDirectory(id, root + "/empty");
                    for (const auto &[path, bytes] : expected) {
                        auto actual = device.read(path);
                        if (actual != bytes)
                            throw std::runtime_error("Device fixture content differs from expected bytes");
                        backups.beginFile(id, path, actual.size());
                        backups.appendFile(id, actual.data(), actual.size());
                        backups.finishFile(id);
                    }
                    backups.finish(id);
                    if (!backups.verify(id))
                        throw std::runtime_error("Fixture backup verification failed");
                    return id;
                } catch (...) {
                    try {
                        backups.interrupt(id, "Fixture backup failed");
                    } catch (...) {
                    }
                    throw;
                }
            };
            auto originalBackup = backup(original, "original-test-fixtures");
            auto changed = original;
            for (auto &[path, bytes] : changed) {
                bytes = "changed-before-restore\n" + path + "\n";
                device.write(path, bytes);
            }
            auto beforeRestore = backup(changed, "automatic-backup-before-test-restore");
            if (!backups.verify(originalBackup) || !backups.verify(beforeRestore))
                throw std::runtime_error("Restore blocked: verified backups required");
            size_t restored = 0;
            for (const auto &entry : backups.entries(originalBackup)) {
                if (entry.directory)
                    continue;
                std::ifstream input(backups.payloadPath(originalBackup, entry), std::ios::binary);
                if (!input || entry.bytes > 4096)
                    throw std::runtime_error("Invalid restore fixture payload");
                std::string data(entry.bytes, '\0');
                input.read(data.data(), data.size());
                if (static_cast<size_t>(input.gcount()) != data.size() || input.peek() != EOF)
                    throw std::runtime_error("Restore fixture payload size changed");
                device.write(entry.path, data);
                if (device.read(entry.path) != original.at(entry.path))
                    throw std::runtime_error("Restored file comparison failed");
                ++restored;
            }
            if (restored != original.size())
                throw std::runtime_error("Restore entry count mismatch");
            device.removeFixtures();
            bool closed = device.closeRpc(console);
            captures.finish(
                recordingId, closed ? CaptureState::Completed : CaptureState::Interrupted,
                closed ? "Fixture backup, pre-restore backup, restore comparison and cleanup confirmed"
                       : "Fixture roundtrip completed; RPC closure not confirmed");
            bool verified = captures.verify(recordingId);
            std::cout << "{\"scope\":\"test-subset\",\"fixture_root\":" << jsonQuote(root)
                      << ",\"original_backup\":" << jsonQuote(originalBackup)
                      << ",\"pre_restore_backup\":" << jsonQuote(beforeRestore)
                      << ",\"restored_files\":" << restored
                      << ",\"comparison_verified\":true,\"fixtures_removed\":true"
                      << ",\"recording_id\":" << jsonQuote(recordingId)
                      << ",\"recording_verified\":" << (verified ? "true" : "false")
                      << ",\"rpc_closed\":" << (closed ? "true" : "false") << "}\n";
            return closed && verified ? 0 : 2;
        } catch (...) {
            wire.setRecording(false);
            try {
                device.closeRpc(console);
            } catch (...) {
            }
            try {
                captures.finish(recordingId, CaptureState::Interrupted,
                                "Fixture roundtrip failed; test directory retained for diagnosis");
            } catch (...) {
            }
            throw;
        }
    } catch (const std::exception &error) {
        std::cerr << error.what() << "\nFixture path: " << root << "\nRecording: " << recordingId << '\n';
        return 1;
    }
}
