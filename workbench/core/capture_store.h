#pragma once
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
namespace UFlipper {
enum class CaptureState { Recording, Completed, Interrupted };
enum class RecordType : unsigned char { Data = 0, Gap = 1, Status = 2 };
struct Capture {
    std::string id, kind, metadata, reason, sha256;
    CaptureState state = CaptureState::Recording;
    uint64_t bytes = 0;
};
class CaptureStore {
  public:
    explicit CaptureStore(const std::filesystem::path &root);
    ~CaptureStore();
    std::string begin(const std::string &kind, const std::string &metadata);
    void append(const std::string &id, const void *bytes, size_t size, uint64_t hostMonotonicNs,
                RecordType type = RecordType::Data);
    void finish(const std::string &id, CaptureState state, const std::string &reason);
    std::vector<Capture> list(size_t limit = 1000, size_t offset = 0) const;
    bool verify(const std::string &id) const;
    std::filesystem::path recordingPath(const std::string &id) const;
    CaptureStore(const CaptureStore &) = delete;
    CaptureStore &operator=(const CaptureStore &) = delete;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
const char *stateName(CaptureState state);
} // namespace UFlipper
