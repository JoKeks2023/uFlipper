#include "key_envelope.h"
#include "io_utils.h"
#include <algorithm>
#include <cstring>
namespace UFlipper {
namespace {
constexpr unsigned char Magic[8] = {'U','F','K','E','Y',0,0,1};
constexpr size_t HeaderSize = 85, CipherSize = 48, EnvelopeSize = HeaderSize + CipherSize;
constexpr size_t IdOffset = 9, OpsOffset = 25, MemoryOffset = 33, AlgorithmOffset = 41, SaltOffset = 45, NonceOffset = 61;
constexpr uint64_t MinimumMemory = 64ULL * 1024 * 1024, MaximumMemory = 256ULL * 1024 * 1024;
class Wipe {
 public:
    explicit Wipe(SecretKey &key) : key_(key) {}
    ~Wipe() { sodium_memzero(key_.data(), key_.size()); }
 private: SecretKey &key_;
};
void number(EnvelopeBytes &bytes, size_t offset, uint64_t n, size_t count) {
    for(size_t i=0;i<count;++i)bytes[offset+count-i-1]=static_cast<unsigned char>(n>>(i*8));
}
uint64_t number(const EnvelopeBytes &bytes, size_t offset, size_t count) {
    uint64_t n=0;for(size_t i=0;i<count;++i)n=(n<<8)|bytes[offset+i];return n;
}
EnvelopeBytes header(UnlockMethod method, const DataKeyId &id) {
    initializeSodium();EnvelopeBytes bytes(EnvelopeSize,0);
    std::copy(std::begin(Magic),std::end(Magic),bytes.begin());bytes[8]=static_cast<unsigned char>(method);
    std::copy(id.begin(),id.end(),bytes.begin()+IdOffset);
    randombytes_buf(bytes.data()+NonceOffset,crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
    if(method==UnlockMethod::Password) {
        number(bytes,OpsOffset,crypto_pwhash_OPSLIMIT_INTERACTIVE,8);
        number(bytes,MemoryOffset,crypto_pwhash_MEMLIMIT_INTERACTIVE,8);
        number(bytes,AlgorithmOffset,crypto_pwhash_ALG_ARGON2ID13,4);
        randombytes_buf(bytes.data()+SaltOffset,crypto_pwhash_SALTBYTES);
    }
    return bytes;
}
void derive(const EnvelopeBytes &bytes,const KeyEnvelopeInfo &info,const std::string &password,SecretKey &key) {
    if(password.empty() || password.size()>1024*1024)throw std::invalid_argument("Password is empty or too long");
    if(crypto_pwhash(key.data(),key.size(),password.data(),password.size(),bytes.data()+SaltOffset,
                     info.operations,static_cast<size_t>(info.memoryBytes),info.algorithm)!=0)
        throw std::runtime_error("Cannot allocate memory for key derivation");
}
void seal(EnvelopeBytes &bytes,const SecretKey &data,const SecretKey &unlock) {
    unsigned long long written=0;
    if(crypto_aead_xchacha20poly1305_ietf_encrypt(bytes.data()+HeaderSize,&written,data.data(),data.size(),
        bytes.data(),HeaderSize,nullptr,bytes.data()+NonceOffset,unlock.data())!=0 || written!=CipherSize)
        throw std::runtime_error("Cannot wrap data key");
}
void open(const EnvelopeBytes &bytes,const DataKeyId &expected,const KeyEnvelopeInfo &info,const SecretKey &unlock,SecretKey &data) {
    if(sodium_memcmp(expected.data(),info.dataKeyId.data(),expected.size())!=0)throw std::runtime_error("Envelope belongs to another data key");
    unsigned long long written=0;
    if(crypto_aead_xchacha20poly1305_ietf_decrypt(data.data(),&written,nullptr,bytes.data()+HeaderSize,CipherSize,
        bytes.data(),HeaderSize,bytes.data()+NonceOffset,unlock.data())!=0 || written!=data.size())
        throw std::runtime_error("Wrong unlock secret or modified key envelope");
}
}
UnlockedDataKey::~UnlockedDataKey() {sodium_memzero(key_.data(),key_.size());}
UnlockedDataKey::UnlockedDataKey(UnlockedDataKey &&other) noexcept : key_(other.key_) {sodium_memzero(other.key_.data(),other.key_.size());}
UnlockedDataKey &UnlockedDataKey::operator=(UnlockedDataKey &&other) noexcept {
    if(this!=&other){sodium_memzero(key_.data(),key_.size());key_=other.key_;sodium_memzero(other.key_.data(),other.key_.size());}return *this;
}
DataKeyId KeyEnvelope::randomId() {initializeSodium();DataKeyId id;randombytes_buf(id.data(),id.size());return id;}
KeyEnvelopeInfo KeyEnvelope::inspect(const EnvelopeBytes &bytes) {
    initializeSodium();
    if(bytes.size()!=EnvelopeSize || std::memcmp(bytes.data(),Magic,sizeof Magic)!=0)throw std::invalid_argument("Unsupported key envelope version or length");
    if(bytes[8]!=1 && bytes[8]!=2)throw std::invalid_argument("Unsupported unlock method");
    KeyEnvelopeInfo info{};info.method=static_cast<UnlockMethod>(bytes[8]);
    std::copy_n(bytes.data()+IdOffset,info.dataKeyId.size(),info.dataKeyId.begin());
    info.operations=number(bytes,OpsOffset,8);info.memoryBytes=number(bytes,MemoryOffset,8);info.algorithm=static_cast<uint32_t>(number(bytes,AlgorithmOffset,4));
    if(info.method==UnlockMethod::Password) {
        // Reject untrusted memory/CPU requests before entering the KDF.
        if(info.algorithm!=crypto_pwhash_ALG_ARGON2ID13 || info.operations<2 || info.operations>6 ||
           info.memoryBytes<MinimumMemory || info.memoryBytes>MaximumMemory)
            throw std::invalid_argument("Unsupported or excessive Argon2id parameters");
    } else if(info.operations || info.memoryBytes || info.algorithm ||
              !std::all_of(bytes.begin()+SaltOffset,bytes.begin()+NonceOffset,[](unsigned char b){return b==0;}))
        throw std::invalid_argument("Unexpected KDF fields in recovery envelope");
    return info;
}
EnvelopeBytes KeyEnvelope::password(const SecretKey &data,const DataKeyId &id,const std::string &password) {
    auto bytes=header(UnlockMethod::Password,id);SecretKey unlock{};Wipe wipe(unlock);
    derive(bytes,inspect(bytes),password,unlock);seal(bytes,data,unlock);return bytes;
}
EnvelopeBytes KeyEnvelope::recovery(const SecretKey &data,const DataKeyId &id,const SecretKey &recovery) {
    auto bytes=header(UnlockMethod::RecoveryKey,id);seal(bytes,data,recovery);return bytes;
}
UnlockedDataKey KeyEnvelope::unlockPassword(const EnvelopeBytes &bytes,const DataKeyId &expected,const std::string &password) {
    const auto info=inspect(bytes);if(info.method!=UnlockMethod::Password)throw std::invalid_argument("Envelope requires a recovery key");
    SecretKey unlock{},data{};Wipe wipeUnlock(unlock),wipeData(data);
    derive(bytes,info,password,unlock);open(bytes,expected,info,unlock,data);return UnlockedDataKey(data);
}
UnlockedDataKey KeyEnvelope::unlockRecovery(const EnvelopeBytes &bytes,const DataKeyId &expected,const SecretKey &recovery) {
    const auto info=inspect(bytes);if(info.method!=UnlockMethod::RecoveryKey)throw std::invalid_argument("Envelope requires a password");
    SecretKey data{};Wipe wipe(data);open(bytes,expected,info,recovery,data);return UnlockedDataKey(data);
}
}
