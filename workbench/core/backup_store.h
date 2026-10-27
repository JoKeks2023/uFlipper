#pragma once
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
namespace UFlipper {
enum class BackupScope { FullDevice, TestSubset };
enum class BackupState { Writing, Verified, Interrupted };
struct BackupInfo {
    std::string id, metadata, reason;
    uint64_t startedMs = 0, endedMs = 0;
    BackupScope scope = BackupScope::TestSubset;
    BackupState state = BackupState::Writing;
};
struct BackupEntry {
    std::string path, sha256, blob;
    uint64_t bytes = 0;
    bool directory = false;
};
class BackupStore {
  public:
    explicit BackupStore(const std::filesystem::path &workbenchRoot);
    ~BackupStore();
    std::string begin(BackupScope scope, const std::string &metadata);
    void addDirectory(const std::string &id, const std::string &devicePath);
    void beginFile(const std::string &id, const std::string &devicePath, uint64_t expectedBytes);
    void appendFile(const std::string &id, const void *bytes, size_t size);
    void finishFile(const std::string &id);
    // The device adapter must compare the final remote inventory before confirming each scope.
    void confirmInventory(const std::string &id, const std::string &storageRoot);
    void finish(const std::string &id);
    void interrupt(const std::string &id, const std::string &reason);
    bool verify(const std::string &id) const;
    BackupInfo get(const std::string &id) const;
    std::vector<BackupInfo> list(size_t limit = 200, size_t offset = 0) const;
    std::vector<BackupEntry> entries(const std::string &id, size_t limit = 200, size_t offset = 0) const;
    std::filesystem::path payloadPath(const std::string &id, const BackupEntry &entry) const;
    BackupStore(const BackupStore &) = delete;
    BackupStore &operator=(const BackupStore &) = delete;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace UFlipper
