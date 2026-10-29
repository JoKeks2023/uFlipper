#pragma once
#include "file_crypto.h"
#include <cstdint>
#include <vector>
namespace UFlipper {
using DataKeyId = std::array<unsigned char, 16>;
using EnvelopeBytes = std::vector<unsigned char>;
enum class UnlockMethod : unsigned char { Password = 1, RecoveryKey = 2 };
struct KeyEnvelopeInfo {
    UnlockMethod method;
    DataKeyId dataKeyId;
    uint64_t operations = 0, memoryBytes = 0;
    uint32_t algorithm = 0;
};
// Move-only, wiping owner of an unwrapped data key. The caller owns the lifetime
// of the password and source/recovery SecretKeys supplied to KeyEnvelope.
class UnlockedDataKey {
 public:
    ~UnlockedDataKey();
    UnlockedDataKey(UnlockedDataKey &&) noexcept;
    UnlockedDataKey &operator=(UnlockedDataKey &&) noexcept;
    UnlockedDataKey(const UnlockedDataKey &) = delete;
    UnlockedDataKey &operator=(const UnlockedDataKey &) = delete;
    const SecretKey &bytes() const { return key_; }
 private:
    friend class KeyEnvelope;
    explicit UnlockedDataKey(const SecretKey &key) : key_(key) {}
    SecretKey key_;
};
class KeyEnvelope {
 public:
    static DataKeyId randomId();
    static EnvelopeBytes password(const SecretKey &dataKey, const DataKeyId &, const std::string &password);
    static EnvelopeBytes recovery(const SecretKey &dataKey, const DataKeyId &, const SecretKey &recoveryKey);
    // This only validates untrusted header structure. Only unlock authenticates it.
    static KeyEnvelopeInfo inspect(const EnvelopeBytes &);
    static UnlockedDataKey unlockPassword(const EnvelopeBytes &, const DataKeyId &expected, const std::string &password);
    static UnlockedDataKey unlockRecovery(const EnvelopeBytes &, const DataKeyId &expected, const SecretKey &recoveryKey);
};
}
