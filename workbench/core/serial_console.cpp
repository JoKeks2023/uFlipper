#include "serial_console.h"
#include <cerrno>
#include <fcntl.h>
#include <poll.h>
#include <stdexcept>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <unistd.h>
namespace UFlipper {
SerialConsole::SerialConsole(const std::string &port) {
    fd_ = ::open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd_ < 0)
        throw std::runtime_error("Cannot open serial port; it may be in use");
    try {
        if (flock(fd_, LOCK_EX | LOCK_NB) != 0 || ioctl(fd_, TIOCEXCL) != 0)
            throw std::runtime_error("Another application owns the serial port");
        if (tcgetattr(fd_, &previous_) != 0)
            throw std::runtime_error("Cannot read serial configuration");
        auto settings = previous_;
        cfmakeraw(&settings);
        settings.c_cflag |= CLOCAL | CREAD;
        settings.c_cc[VMIN] = 0;
        settings.c_cc[VTIME] = 0;
        cfsetispeed(&settings, B115200);
        cfsetospeed(&settings, B115200);
        if (tcsetattr(fd_, TCSANOW, &settings) != 0)
            throw std::runtime_error("Cannot configure serial port");
        configured_ = true;
    } catch (...) {
        ioctl(fd_, TIOCNXCL);
        ::close(fd_);
        fd_ = -1;
        throw;
    }
}
SerialConsole::~SerialConsole() {
    if (fd_ >= 0) {
        if (configured_)
            tcsetattr(fd_, TCSANOW, &previous_);
        ioctl(fd_, TIOCNXCL);
        ::close(fd_);
    }
}
void SerialConsole::send(const std::string &bytes) {
    size_t offset = 0;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (offset < bytes.size()) {
        if (std::chrono::steady_clock::now() > deadline)
            throw std::runtime_error("Serial write timed out");
        auto count = ::write(fd_, bytes.data() + offset, bytes.size() - offset);
        if (count > 0) {
            offset += count;
            continue;
        }
        if (count < 0 && (errno == EAGAIN || errno == EINTR)) {
            pollfd wait{fd_, POLLOUT, 0};
            poll(&wait, 1, 50);
            continue;
        }
        throw std::runtime_error("Serial port disconnected while writing");
    }
}
std::string SerialConsole::read(std::chrono::milliseconds timeout) {
    pollfd wait{fd_, POLLIN, 0};
    auto ready = poll(&wait, 1, static_cast<int>(timeout.count()));
    if (ready < 0 && errno == EINTR)
        return {};
    if (ready < 0 || (wait.revents & (POLLERR | POLLHUP | POLLNVAL)))
        throw std::runtime_error("Serial port disconnected");
    if (ready == 0)
        return {};
    char bytes[16384];
    auto count = ::read(fd_, bytes, sizeof bytes);
    if (count < 0 && (errno == EAGAIN || errno == EINTR))
        return {};
    if (count <= 0)
        throw std::runtime_error("Serial stream ended");
    return std::string(bytes, count);
}
bool SerialConsole::waitForPrompt(std::chrono::milliseconds timeout, const Sink &sink) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    std::string tail;
    while (std::chrono::steady_clock::now() < deadline) {
        auto bytes = read(std::chrono::milliseconds(50));
        if (sink && !bytes.empty())
            sink(bytes);
        tail += bytes;
        if (tail.size() > 32)
            tail.erase(0, tail.size() - 32);
        const std::string prompt = "\r\n>: ";
        if (tail.size() >= prompt.size() && tail.compare(tail.size() - prompt.size(), prompt.size(), prompt) == 0)
            return true;
    }
    return false;
}
bool SerialConsole::stop(std::chrono::milliseconds timeout, const Sink &sink) {
    send(std::string(1, '\x03'));
    return waitForPrompt(timeout, sink);
}
} // namespace UFlipper
