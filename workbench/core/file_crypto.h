#pragma once
#include <array>
#include <filesystem>
#include <string>
namespace UFlipper {
using SecretKey = std::array<unsigned char, 32>;
using PasswordSalt = std::array<unsigned char, 16>;
class FileCrypto {
  public:
    static SecretKey randomKey();
    static PasswordSalt randomSalt();
    static SecretKey deriveKey(const std::string &password, const PasswordSalt &salt);
    static std::string encodeKey(const SecretKey &key);
    static SecretKey decodeKey(const std::string &encoded);
    static void encrypt(const std::filesystem::path &source, const std::filesystem::path &target,
                        const SecretKey &key);
    static void decrypt(const std::filesystem::path &source, const std::filesystem::path &target,
                        const SecretKey &key);
};
} // namespace UFlipper
