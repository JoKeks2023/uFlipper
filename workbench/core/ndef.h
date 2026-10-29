#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace UFlipper {
using NfcBytes = std::vector<uint8_t>;
struct NdefRecord { uint8_t tnf; NfcBytes type, id, payload; };
// Bounded record parser. Chunked messages are rejected explicitly, not partly shown.
std::vector<NdefRecord> parseNdef(const NfcBytes &message);
class Type4NdefReader {
 public:
    explicit Type4NdefReader(unsigned frameLimit = 240);
    NfcBytes command() const;
    std::string label() const;
    void accept(const NfcBytes &response);
    bool complete() const { return stage_ == Done; }
    const NfcBytes &message() const { return message_; }
 private:
    enum Stage { SelectApplication, SelectCC, ReadCCLength, ReadCC, SelectNdef, ReadLength, ReadMessage, CheckLength, Done };
    void parseCC();
    unsigned readSize() const;
    Stage stage_ = SelectApplication;
    unsigned chunk_, ccLength_ = 0, mle_ = 0, file_ = 0, fileSize_ = 0, length_ = 0;
    NfcBytes cc_, message_;
};
}
