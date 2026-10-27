#include "serial_console.h"
#include <chrono>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <unistd.h>
#ifdef __APPLE__
#include <util.h>
#else
#include <pty.h>
#endif
using namespace UFlipper;
static void require(bool condition, const char *why) {
    if (!condition)
        throw std::runtime_error(why);
}
int main() {
    int master = -1, slave = -1;
    char name[256] = {};
    if (openpty(&master, &slave, name, nullptr, nullptr) != 0)
        return 1;
    try {
        {
            SerialConsole console(name);
            std::string collected;
            std::thread device([&] {
                char command[64];
                auto n = ::read(master, command, sizeof command);
                if (n > 0) {
                    const char a[] = "RAW, 4 samples:\r\n100 200 100 200\r\n";
                    ::write(master, a, sizeof(a) - 1);
                    char stop = 0;
                    ::read(master, &stop, 1);
                    if (stop == '\x03') {
                        ::write(master, "\r\n>", 3);
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        ::write(master, ": ", 2);
                    }
                }
            });
            console.send("ir rx raw\r");
            collected += console.read(std::chrono::seconds(1));
            bool stopped =
                console.stop(std::chrono::seconds(1), [&](const std::string &bytes) { collected += bytes; });
            device.join();
            require(stopped, "split stop acknowledgement not recognized");
            require(collected.find("100 200") != std::string::npos, "original bytes lost");
            require(!console.waitForPrompt(std::chrono::milliseconds(70)),
                    "timeout reported as stop acknowledgement");
        }
        ::close(master);
        ::close(slave);
        std::cout << "PASS: serial bytes, split prompt and stop timeout\n";
        return 0;
    } catch (const std::exception &e) {
        if (master >= 0)
            ::close(master);
        if (slave >= 0)
            ::close(slave);
        std::cerr << e.what() << "\n";
        return 1;
    }
}
