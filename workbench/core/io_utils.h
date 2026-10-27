#pragma once
#include <cerrno>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <sodium.h>
#include <stdexcept>
#include <string>
#include <unistd.h>
namespace UFlipper {
inline void initializeSodium() {
    if (sodium_init() < 0)
        throw std::runtime_error("Cannot initialize cryptography");
}
inline std::string randomId() {
    initializeSodium();
    unsigned char bytes[16];
    char hex[33];
    randombytes_buf(bytes, sizeof bytes);
    sodium_bin2hex(hex, sizeof hex, bytes, sizeof bytes);
    return hex;
}
inline void writeAll(int fd, const void *data, size_t size) {
    auto *bytes = static_cast<const unsigned char *>(data);
    while (size) {
        auto count = ::write(fd, bytes, size);
        if (count < 0 && errno == EINTR)
            continue;
        if (count <= 0)
            throw std::runtime_error("Cannot write recording: storage error");
        bytes += count;
        size -= static_cast<size_t>(count);
    }
}
inline void syncFile(int fd) {
    if (::fsync(fd) != 0)
        throw std::runtime_error("Cannot flush data to storage");
}
inline std::string hashFile(const std::filesystem::path &path) {
    std::ifstream input(path, std::ios::binary);
    if (!input)
        throw std::runtime_error("Cannot open file for integrity check");
    crypto_hash_sha256_state state;
    crypto_hash_sha256_init(&state);
    char buffer[65536];
    while (input) {
        input.read(buffer, sizeof buffer);
        crypto_hash_sha256_update(&state, reinterpret_cast<const unsigned char *>(buffer), input.gcount());
    }
    if (input.bad())
        throw std::runtime_error("Cannot read file for integrity check");
    unsigned char digest[32];
    char hex[65];
    crypto_hash_sha256_final(&state, digest);
    sodium_bin2hex(hex, sizeof hex, digest, sizeof digest);
    return hex;
}
inline std::string jsonQuote(const std::string &value) {
    std::string out = "\"";
    const char *hex = "0123456789abcdef";
    for (unsigned char c : value) {
        if (c == '"' || c == '\\') {
            out += '\\';
            out += c;
        } else if (c < 32) {
            out += "\\u00";
            out += hex[c >> 4];
            out += hex[c & 15];
        } else
            out += c;
    }
    return out + '"';
}
class AtomicOutput {
  public:
    explicit AtomicOutput(const std::filesystem::path &target)
        : target_(target), temporary_(target.string() + ".part-" + randomId()) {
        fd_ = ::open(temporary_.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
        if (fd_ < 0)
            throw std::runtime_error("Cannot create output file");
    }
    ~AtomicOutput() {
        if (fd_ >= 0)
            ::close(fd_);
        std::error_code error;
        std::filesystem::remove(temporary_, error);
    }
    int fd() const {
        return fd_;
    }
    void publish(bool replace = false) {
        syncFile(fd_);
        ::close(fd_);
        fd_ = -1;
        if (replace)
            std::filesystem::rename(temporary_, target_);
        else {
            if (::link(temporary_.c_str(), target_.c_str()) != 0)
                throw std::runtime_error("Output exists or cannot be published");
            std::filesystem::remove(temporary_);
        }
    }
    AtomicOutput(const AtomicOutput &) = delete;
    AtomicOutput &operator=(const AtomicOutput &) = delete;

  private:
    std::filesystem::path target_, temporary_;
    int fd_ = -1;
};
} // namespace UFlipper
