#include "stdiorelay.h"
#include "localgateway.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QSocketNotifier>
#include <QTimer>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
namespace Mcp {
namespace {
class Relay : public QObject {
 public:
    Relay(const QJsonObject &endpoint) : input_(STDIN_FILENO, QSocketNotifier::Read),
                                        output_(STDOUT_FILENO, QSocketNotifier::Write) {
        input_.setEnabled(false); output_.setEnabled(false);
        socket_.setReadBufferSize(MaxMessage + 1);
        deadline_.setSingleShot(true);
        connect(&deadline_, &QTimer::timeout, this, [this] { fail("The local MCP gateway did not respond."); });
        connect(&input_, &QSocketNotifier::activated, this, [this] { readInput(); });
        connect(&output_, &QSocketNotifier::activated, this, [this] { writeOutput(); });
        connect(&socket_, &QLocalSocket::connected, this, [this, endpoint] {
            socket_.write(QJsonDocument(QJsonObject{{"token", endpoint.value("token")}}).toJson(QJsonDocument::Compact) + '\n');
        });
        connect(&socket_, &QLocalSocket::readyRead, this, [this] { readSocket(); });
        connect(&socket_, &QLocalSocket::disconnected, this, [this] {
            input_.setEnabled(false); closing_ = true;
            if (out_.isEmpty()) QCoreApplication::exit(0);
        });
        connect(&socket_, &QLocalSocket::errorOccurred, this, [this](QLocalSocket::LocalSocketError) {
            if (!closing_) fail("uFlipper is unavailable or MCP permission was revoked. No device action was retried.");
        });
        deadline_.start(5000);
        socket_.connectToServer(endpoint.value("socket").toString());
    }
 private:
    void fail(const char *message) {
        std::fprintf(stderr, "%s\n", message);
        QCoreApplication::exit(2);
    }
    void readInput() {
        char bytes[8192];
        const auto count = ::read(STDIN_FILENO, bytes, sizeof(bytes));
        if (!count) {
            input_.setEnabled(false); closing_ = true;
            socket_.disconnectFromServer();
            if (socket_.state() == QLocalSocket::UnconnectedState && out_.isEmpty()) QCoreApplication::exit(0);
            return;
        }
        if (count < 0) { if (errno != EAGAIN && errno != EINTR) fail("Cannot read MCP stdin."); return; }
        in_.append(bytes, count);
        while (true) {
            const auto newline = in_.indexOf('\n');
            if (newline < 0) { if (in_.size() > MaxMessage) fail("MCP request exceeds 64 KiB."); break; }
            if (newline > MaxMessage || socket_.bytesToWrite() + newline + 1 > MaxBuffered) {
                fail("MCP input exceeded the bounded local transport queue."); return;
            }
            socket_.write(in_.left(newline + 1)); in_.remove(0, newline + 1);
        }
    }
    void readSocket() {
        wire_ += socket_.readAll();
        if (!authenticated_) {
            const auto newline = wire_.indexOf('\n');
            if (newline < 0) { if (wire_.size() > 4096) fail("Invalid local gateway greeting."); return; }
            const auto greeting = QJsonDocument::fromJson(wire_.left(newline)).object();
            if (greeting.value("authenticated") != true) { fail("Local MCP authentication failed."); return; }
            wire_.remove(0, newline + 1); authenticated_ = true; deadline_.stop(); input_.setEnabled(true);
        }
        if (out_.size() + wire_.size() > MaxBuffered) { fail("MCP client is not reading responses."); return; }
        out_ += wire_; wire_.clear();
        writeOutput();
    }
    void writeOutput() {
        while (!out_.isEmpty()) {
            const auto count = ::write(STDOUT_FILENO, out_.constData(), size_t(out_.size()));
            if (count < 0) {
                if (errno == EINTR) continue;
                if (errno == EAGAIN) { output_.setEnabled(true); return; }
                fail("Cannot write MCP stdout."); return;
            }
            out_.remove(0, count);
        }
        output_.setEnabled(false);
        if (closing_ && socket_.state() == QLocalSocket::UnconnectedState) QCoreApplication::exit(0);
    }
    QLocalSocket socket_;
    QSocketNotifier input_, output_;
    QTimer deadline_;
    QByteArray in_, out_, wire_;
    bool authenticated_ = false, closing_ = false;
};
}
int runStdioRelay(const QString &directory) {
    const auto endpoint = readPrivateJson(directory + "/endpoint.json");
    if (endpoint.value("schema") != 1 || !endpoint.value("socket").isString() || endpoint.value("token").toString().size() != 64) {
        std::fprintf(stderr, "MCP is disabled. Open uFlipper and explicitly grant access in Settings > Integrations.\n");
        return 2;
    }
    std::signal(SIGPIPE, SIG_IGN);
    const auto inFlags = ::fcntl(STDIN_FILENO, F_GETFL);
    const auto outFlags = ::fcntl(STDOUT_FILENO, F_GETFL);
    if (inFlags < 0 || outFlags < 0 || ::fcntl(STDIN_FILENO, F_SETFL, inFlags | O_NONBLOCK) < 0 ||
        ::fcntl(STDOUT_FILENO, F_SETFL, outFlags | O_NONBLOCK) < 0) {
        std::fprintf(stderr, "Cannot configure the local MCP standard streams.\n"); return 2;
    }
    Relay relay(endpoint);
    return QCoreApplication::exec();
}
}
