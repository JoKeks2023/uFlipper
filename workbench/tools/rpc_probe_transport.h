#pragma once
// Bounded development transport shared by physical-device probes.
// These development probes do not replace the production Qt connection owner.
#include "capture_store.h"
#include "flipper.pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "serial_console.h"
#include <chrono>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
namespace UFlipper::Probe {
using Clock = std::chrono::steady_clock;
struct Decoded {
    PB_Main message = PB_Main_init_zero;
    ~Decoded() {
        pb_release(&PB_Main_msg, &message);
    }
};
class Wire {
  public:
    Wire(SerialConsole &console, std::function<void(const std::string &, RecordType)> record)
        : console_(console), record_(std::move(record)) {}
    void setRecording(bool enabled) {
        recording_ = enabled;
    }
    void send(const PB_Main &message) {
        std::vector<unsigned char> bytes(8192);
        auto stream = pb_ostream_from_buffer(bytes.data(), bytes.size());
        if (!pb_encode_ex(&stream, &PB_Main_msg, &message, PB_ENCODE_DELIMITED))
            throw std::runtime_error("Cannot encode bounded RPC request");
        std::string encoded(reinterpret_cast<char *>(bytes.data()), stream.bytes_written);
        if (recording_)
            record_(encoded, RecordType::Status);
        console_.send(encoded);
    }
    std::unique_ptr<Decoded> next(Clock::time_point deadline) {
        while (Clock::now() < deadline) {
            uint32_t size = 0;
            size_t prefix = 0;
            bool complete = false;
            for (size_t i = 0; i < buffer_.size() && i < 5; ++i) {
                auto byte = static_cast<unsigned char>(buffer_[i]);
                if (i == 4 && (byte & 0xf0))
                    throw std::runtime_error("RPC size prefix overflow");
                size |= uint32_t(byte & 0x7f) << (i * 7);
                prefix = i + 1;
                if (!(byte & 0x80)) {
                    complete = true;
                    break;
                }
            }
            if (!complete && buffer_.size() >= 5)
                throw std::runtime_error("Invalid RPC size prefix");
            if (complete) {
                if (size > 8192)
                    throw std::runtime_error("RPC frame exceeds probe limit");
                if (buffer_.size() >= prefix + size) {
                    auto result = std::make_unique<Decoded>();
                    auto stream = pb_istream_from_buffer(reinterpret_cast<const pb_byte_t *>(buffer_.data()),
                                                         prefix + size);
                    if (!pb_decode_ex(&stream, &PB_Main_msg, &result->message, PB_DECODE_DELIMITED))
                        throw std::runtime_error("Cannot decode RPC response");
                    buffer_.erase(0, prefix + size);
                    return result;
                }
            }
            auto bytes = console_.read(std::chrono::milliseconds(50));
            if (recording_ && !bytes.empty())
                record_(bytes, RecordType::Data);
            buffer_ += bytes;
            if (buffer_.size() > 65536)
                throw std::runtime_error("RPC receive buffer limit reached");
        }
        throw std::runtime_error("RPC response timed out");
    }

  private:
    SerialConsole &console_;
    std::function<void(const std::string &, RecordType)> record_;
    std::string buffer_;
    bool recording_ = true;
};
inline PB_Main request(uint32_t id, pb_size_t tag) {
    PB_Main message = PB_Main_init_zero;
    message.command_id = id;
    message.which_content = tag;
    return message;
}
} // namespace UFlipper::Probe
