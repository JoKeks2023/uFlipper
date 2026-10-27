#include "file_crypto.h"
#include "io_utils.h"
#include <cstring>
#include <vector>
namespace UFlipper {
namespace {
class WipeOnExit {
  public:
    WipeOnExit(void *bytes, size_t size) : bytes_(bytes), size_(size) {}
    ~WipeOnExit() {
        sodium_memzero(bytes_, size_);
    }
    WipeOnExit(const WipeOnExit &) = delete;
    WipeOnExit &operator=(const WipeOnExit &) = delete;

  private:
    void *bytes_;
    size_t size_;
};
} // namespace
static constexpr size_t ChunkSize = 65536;
static constexpr unsigned char Magic[8] = {'U', 'F', 'E', 'N', 'C', 0, 0, 1};
SecretKey FileCrypto::randomKey() {
    initializeSodium();
    SecretKey key;
    randombytes_buf(key.data(), key.size());
    return key;
}
PasswordSalt FileCrypto::randomSalt() {
    initializeSodium();
    PasswordSalt salt;
    randombytes_buf(salt.data(), salt.size());
    return salt;
}
SecretKey FileCrypto::deriveKey(const std::string &password, const PasswordSalt &salt) {
    initializeSodium();
    if (password.empty())
        throw std::invalid_argument("Password is required");
    SecretKey key;
    if (crypto_pwhash(key.data(), key.size(), password.data(), password.size(), salt.data(),
                      crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
                      crypto_pwhash_ALG_ARGON2ID13) != 0)
        throw std::runtime_error("Not enough memory to derive encryption key");
    return key;
}
std::string FileCrypto::encodeKey(const SecretKey &key) {
    initializeSodium();
    char out[sodium_base64_ENCODED_LEN(32, sodium_base64_VARIANT_URLSAFE_NO_PADDING)];
    sodium_bin2base64(out, sizeof out, key.data(), key.size(), sodium_base64_VARIANT_URLSAFE_NO_PADDING);
    return std::string("ufkey1:") + out;
}
SecretKey FileCrypto::decodeKey(const std::string &encoded) {
    initializeSodium();
    if (encoded.rfind("ufkey1:", 0) != 0)
        throw std::invalid_argument("Unsupported recovery key format");
    SecretKey key;
    size_t size = 0;
    const char *end = nullptr;
    if (sodium_base642bin(key.data(), key.size(), encoded.data() + 7, encoded.size() - 7, nullptr, &size,
                          &end, sodium_base64_VARIANT_URLSAFE_NO_PADDING) != 0 ||
        size != key.size() || end != encoded.data() + encoded.size())
        throw std::invalid_argument("Invalid recovery key");
    return key;
}
static void lengthPrefix(int fd, uint32_t size) {
    unsigned char encoded[4] = {static_cast<unsigned char>(size >> 24),
                                static_cast<unsigned char>(size >> 16), static_cast<unsigned char>(size >> 8),
                                static_cast<unsigned char>(size)};
    writeAll(fd, encoded, sizeof encoded);
}
static void encryptChunk(int fd, crypto_secretstream_xchacha20poly1305_state &state,
                         const unsigned char *data, size_t size, unsigned char tag) {
    std::vector<unsigned char> cipher(size + crypto_secretstream_xchacha20poly1305_ABYTES);
    unsigned long long written = 0;
    crypto_secretstream_xchacha20poly1305_push(&state, cipher.data(), &written, data, size, nullptr, 0, tag);
    lengthPrefix(fd, static_cast<uint32_t>(written));
    writeAll(fd, cipher.data(), written);
}
void FileCrypto::encrypt(const std::filesystem::path &source, const std::filesystem::path &target,
                         const SecretKey &key) {
    initializeSodium();
    std::ifstream input(source, std::ios::binary);
    if (!input)
        throw std::runtime_error("Cannot read source file");
    AtomicOutput output(target);
    crypto_secretstream_xchacha20poly1305_state state{};
    WipeOnExit wipeState(&state, sizeof state);
    unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    crypto_secretstream_xchacha20poly1305_init_push(&state, header, key.data());
    writeAll(output.fd(), Magic, sizeof Magic);
    writeAll(output.fd(), header, sizeof header);
    unsigned char buffer[ChunkSize];
    WipeOnExit wipeBuffer(buffer, sizeof buffer);
    while (input) {
        input.read(reinterpret_cast<char *>(buffer), sizeof buffer);
        auto size = input.gcount();
        if (size)
            encryptChunk(output.fd(), state, buffer, size, 0);
    }
    if (input.bad())
        throw std::runtime_error("Cannot read source file");
    encryptChunk(output.fd(), state, nullptr, 0, crypto_secretstream_xchacha20poly1305_TAG_FINAL);
    output.publish();
}
void FileCrypto::decrypt(const std::filesystem::path &source, const std::filesystem::path &target,
                         const SecretKey &key) {
    initializeSodium();
    std::ifstream input(source, std::ios::binary);
    if (!input)
        throw std::runtime_error("Cannot read encrypted file");
    unsigned char magic[8], header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    if (!input.read(reinterpret_cast<char *>(magic), sizeof magic) ||
        std::memcmp(magic, Magic, sizeof magic) != 0 ||
        !input.read(reinterpret_cast<char *>(header), sizeof header))
        throw std::runtime_error("Unsupported or truncated encrypted file");
    crypto_secretstream_xchacha20poly1305_state state{};
    WipeOnExit wipeState(&state, sizeof state);
    if (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, key.data()) != 0)
        throw std::runtime_error("Invalid encryption header");
    AtomicOutput output(target);
    bool final = false;
    while (!final) {
        unsigned char prefix[4];
        if (!input.read(reinterpret_cast<char *>(prefix), sizeof prefix))
            throw std::runtime_error("Encrypted stream has no final tag");
        uint32_t size = (uint32_t(prefix[0]) << 24) | (uint32_t(prefix[1]) << 16) |
                        (uint32_t(prefix[2]) << 8) | prefix[3];
        if (size < crypto_secretstream_xchacha20poly1305_ABYTES ||
            size > ChunkSize + crypto_secretstream_xchacha20poly1305_ABYTES)
            throw std::runtime_error("Invalid encrypted chunk length");
        std::vector<unsigned char> cipher(size), plain(size);
        WipeOnExit wipePlain(plain.data(), plain.size());
        if (!input.read(reinterpret_cast<char *>(cipher.data()), size))
            throw std::runtime_error("Truncated encrypted chunk");
        unsigned long long written = 0;
        unsigned char tag = 0;
        if (crypto_secretstream_xchacha20poly1305_pull(&state, plain.data(), &written, &tag, cipher.data(),
                                                       size, nullptr, 0) != 0)
            throw std::runtime_error("Wrong key or modified encrypted file");
        writeAll(output.fd(), plain.data(), written);
        final = tag == crypto_secretstream_xchacha20poly1305_TAG_FINAL;
    }
    if (input.peek() != std::char_traits<char>::eof())
        throw std::runtime_error("Unexpected data after encrypted stream");
    output.publish();
}
} // namespace UFlipper
