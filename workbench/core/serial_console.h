#pragma once
#include <chrono>
#include <functional>
#include <string>
#include <termios.h>
namespace UFlipper {
class SerialConsole {
  public:
    using Sink = std::function<void(const std::string &)>;
    explicit SerialConsole(const std::string &port);
    ~SerialConsole();
    void send(const std::string &bytes);
    std::string read(std::chrono::milliseconds timeout);
    bool waitForPrompt(std::chrono::milliseconds timeout, const Sink &sink = {});
    bool stop(std::chrono::milliseconds timeout, const Sink &sink = {});
    SerialConsole(const SerialConsole &) = delete;
    SerialConsole &operator=(const SerialConsole &) = delete;

  private:
    int fd_ = -1;
    termios previous_{};
    bool configured_ = false;
};
} // namespace UFlipper
