#include "operation_coordinator.h"
#include <algorithm>
#include <sodium.h>
#include <stdexcept>
namespace UFlipper {
const char *stateName(JobState state) {
    switch (state) {
    case JobState::AwaitingConfirmation:
        return "awaiting_confirmation";
    case JobState::Waiting:
        return "waiting";
    case JobState::Starting:
        return "starting";
    case JobState::Running:
        return "running";
    case JobState::Stopping:
        return "stopping";
    case JobState::Completed:
        return "completed";
    case JobState::Canceled:
        return "canceled";
    case JobState::Failed:
        return "failed";
    case JobState::Unknown:
        return "unknown";
    }
    return "unknown";
}
void OperationCoordinator::connectDevice(const std::string &identity) {
    if (identity.empty())
        throw std::invalid_argument("Device identity is required");
    disconnectDevice("Device generation changed");
    std::lock_guard<std::mutex> lock(mutex_);
    device_ = identity;
    ++generation_;
    if (nonce_.empty()) {
        if (sodium_init() < 0)
            throw std::runtime_error("Cannot initialize cryptographic runtime");
        unsigned char data[8];
        char encoded[17];
        randombytes_buf(data, sizeof data);
        sodium_bin2hex(encoded, sizeof encoded, data, sizeof data);
        nonce_ = encoded;
    }
}
void OperationCoordinator::disconnectDevice(const std::string &reason) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &pair : jobs_) {
        auto &job = pair.second;
        if (ownsHardware(job)) {
            job.state = JobState::Unknown;
            job.reason = reason;
        } else if (job.state == JobState::Waiting || job.state == JobState::AwaitingConfirmation) {
            job.state = JobState::Canceled;
            job.reason = reason;
        }
    }
    device_.clear();
}
bool OperationCoordinator::ownsHardware(const Job &job) const {
    return job.generation == generation_ &&
           (job.state == JobState::Starting || job.state == JobState::Running ||
            job.state == JobState::Stopping || job.state == JobState::Unknown);
}
std::vector<std::string> OperationCoordinator::conflicts(uint64_t resources) const {
    std::vector<std::string> result;
    for (const auto &pair : jobs_)
        if (ownsHardware(pair.second) && (pair.second.resources & resources))
            result.push_back(pair.first);
    return result;
}
Request OperationCoordinator::request(const std::string &kind, uint64_t resources, Origin origin, bool live,
                                      bool recordingReady) {
    if (kind.empty() || resources == 0 || (resources & ~uint64_t(1023)))
        throw std::invalid_argument("Valid operation and resources are required");
    std::lock_guard<std::mutex> lock(mutex_);
    if (device_.empty())
        return {{}, Decision::Disconnected, {}};
    if (live && !recordingReady)
        return {{}, Decision::RecordingUnavailable, {}};
    if (jobs_.size() >= 512) {
        auto old = std::find_if(order_.begin(), order_.end(), [&](const std::string &id) {
            auto s = jobs_.at(id).state;
            return !ownsHardware(jobs_.at(id)) && (s == JobState::Completed || s == JobState::Canceled ||
                                                   s == JobState::Failed || s == JobState::Unknown);
        });
        if (old == order_.end())
            throw std::runtime_error("Too many pending operations");
        jobs_.erase(*old);
        order_.erase(old);
    }
    auto blocked = conflicts(resources);
    Job job;
    job.id = nonce_ + "-" + std::to_string(++sequence_);
    job.kind = kind;
    job.resources = resources;
    job.origin = origin;
    job.live = live;
    job.generation = generation_;
    job.state = blocked.empty() ? JobState::Starting : JobState::AwaitingConfirmation;
    jobs_.emplace(job.id, job);
    order_.push_back(job.id);
    return {job.id, blocked.empty() ? Decision::Granted : Decision::NeedsConfirmation, blocked};
}
bool OperationCoordinator::resolveConflict(const std::string &id, bool accept) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = jobs_.find(id);
    if (it == jobs_.end() || it->second.state != JobState::AwaitingConfirmation ||
        it->second.generation != generation_)
        return false;
    if (!accept) {
        it->second.state = JobState::Canceled;
        it->second.reason = "Conflict declined";
        return true;
    }
    it->second.state = JobState::Waiting;
    for (const auto &conflict : conflicts(it->second.resources))
        jobs_.at(conflict).state = JobState::Stopping;
    startWaiting();
    return true;
}
void OperationCoordinator::startWaiting() {
    if (device_.empty())
        return;
    for (const auto &id : order_) {
        auto &job = jobs_.at(id);
        if (job.state == JobState::Waiting && job.generation == generation_ &&
            conflicts(job.resources).empty())
            job.state = JobState::Starting;
    }
}
bool OperationCoordinator::markRunning(const std::string &id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = jobs_.find(id);
    if (it == jobs_.end() || it->second.state != JobState::Starting || it->second.generation != generation_ ||
        device_.empty())
        return false;
    it->second.state = JobState::Running;
    return true;
}
bool OperationCoordinator::requestStop(const std::string &id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = jobs_.find(id);
    if (it == jobs_.end() || !ownsHardware(it->second))
        return false;
    it->second.state = JobState::Stopping;
    return true;
}
bool OperationCoordinator::acknowledgeStop(const std::string &id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = jobs_.find(id);
    if (it == jobs_.end() || it->second.state != JobState::Stopping)
        return false;
    it->second.state = JobState::Completed;
    it->second.reason = "Stop confirmed";
    startWaiting();
    return true;
}
bool OperationCoordinator::finish(const std::string &id, JobState state, const std::string &reason) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = jobs_.find(id);
    if (it == jobs_.end() || !ownsHardware(it->second))
        return false;
    it->second.state = state;
    it->second.reason = reason;
    startWaiting();
    return true;
}
bool OperationCoordinator::complete(const std::string &id) {
    return finish(id, JobState::Completed, "Completed");
}
bool OperationCoordinator::fail(const std::string &id, const std::string &reason, bool resourcesReleased) {
    return finish(id, resourcesReleased ? JobState::Failed : JobState::Unknown, reason);
}
Job OperationCoordinator::get(const std::string &id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return jobs_.at(id);
}
std::vector<Job> OperationCoordinator::jobs() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Job> result;
    for (const auto &id : order_)
        result.push_back(jobs_.at(id));
    return result;
}
} // namespace UFlipper
