#pragma once
#include <cstddef>
#include <string>
namespace UFlipper {
// Only firmware acknowledgements change the receive state; echoed commands are not evidence.
class ReceiveStatus {
  public:
    explicit ReceiveStatus(const std::string &kind);
    void feed(const std::string &bytes);
    bool started() const {
        return started_;
    }
    bool returnedToPrompt() const {
        return prompt_;
    }
    size_t irFrames() const {
        return frames_;
    }

  private:
    std::string banner_, line_, tail_;
    bool started_ = false, prompt_ = false;
    size_t frames_ = 0;
};
} // namespace UFlipper
