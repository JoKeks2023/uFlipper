#include "receive_status.h"
#include <stdexcept>
namespace UFlipper {
ReceiveStatus::ReceiveStatus(const std::string &kind) {
    if (kind == "ir.raw")
        banner_ = "Receiving RAW INFRARED...";
    else if (kind == "ir.decoded")
        banner_ = "Receiving  INFRARED...";
    else if (kind == "subghz.raw")
        banner_ = "Listening at ";
    else if (kind == "subghz.decoded")
        banner_ = "Listening at frequency: ";
    else
        throw std::invalid_argument("Unsupported receive format");
}
void ReceiveStatus::feed(const std::string &bytes) {
    for (char c : bytes) {
        tail_ += c;
        if (tail_.size() > 5)
            tail_.erase(0, tail_.size() - 5);
        if (tail_ == "\r\n>: ")
            prompt_ = true;
        if (c == '\n') {
            if (!line_.empty() && line_.back() == '\r')
                line_.pop_back();
            if (line_.rfind(banner_, 0) == 0)
                started_ = true;
            if (started_ && (line_.rfind("RAW, ", 0) == 0 || line_.find(", A:0x") != std::string::npos))
                ++frames_;
            line_.clear();
        } else {
            line_ += c;
            if (line_.size() > 8192)
                line_.erase(0, line_.size() - 8192);
        }
    }
}
} // namespace UFlipper
