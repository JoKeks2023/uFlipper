#pragma once
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>
namespace UFlipper {
enum Resource : uint64_t {
    Control = 1,
    DeviceApp = 2,
    Infrared = 4,
    Radio = 8,
    Nfc = 16,
    LowFrequency = 32,
    Gpio = 64,
    Storage = 128,
    UsbMode = 256,
    BluetoothMode = 512
};
enum class Origin { Ui, Mcp, Reader };
enum class JobState {
    AwaitingConfirmation,
    Waiting,
    Starting,
    Running,
    Stopping,
    Completed,
    Canceled,
    Failed,
    Unknown
};
enum class Decision { Granted, NeedsConfirmation, Disconnected, RecordingUnavailable };
struct Job {
    std::string id, kind, reason;
    uint64_t resources = 0, generation = 0;
    Origin origin = Origin::Ui;
    JobState state = JobState::Starting;
    bool live = false;
};
struct Request {
    std::string id;
    Decision decision;
    std::vector<std::string> conflicts;
};
class OperationCoordinator {
  public:
    void connectDevice(const std::string &identity);
    void disconnectDevice(const std::string &reason);
    Request request(const std::string &kind, uint64_t resources, Origin origin, bool live = false,
                    bool recordingReady = false);
    bool resolveConflict(const std::string &id, bool accept);
    bool markRunning(const std::string &id);
    bool requestStop(const std::string &id);
    bool acknowledgeStop(const std::string &id);
    bool complete(const std::string &id);
    bool fail(const std::string &id, const std::string &reason, bool resourcesReleased = false);
    Job get(const std::string &id) const;
    std::vector<Job> jobs() const;

  private:
    bool finish(const std::string &id, JobState state, const std::string &reason);
    bool ownsHardware(const Job &job) const;
    std::vector<std::string> conflicts(uint64_t resources) const;
    void startWaiting();
    mutable std::mutex mutex_;
    std::string device_, nonce_;
    uint64_t generation_ = 0, sequence_ = 0;
    std::map<std::string, Job> jobs_;
    std::vector<std::string> order_;
};
const char *stateName(JobState state);
} // namespace UFlipper
