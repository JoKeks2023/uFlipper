#include "localgateway.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QRandomGenerator>
#include <QSaveFile>
#include <QStandardPaths>
#include <QSysInfo>
#include <sys/stat.h>
#include <unistd.h>
namespace Mcp {
namespace {
constexpr auto privateFile = QFileDevice::ReadOwner | QFileDevice::WriteOwner;
QString nonce() {
    QByteArray bytes(32, '\0');
    auto *rng = QRandomGenerator::system();
    for (int i = 0; i < bytes.size(); ++i) bytes[i] = char(rng->generate() & 255);
    return QString::fromLatin1(bytes.toHex());
}
}
QString machineIdentity() {
    const auto value = QSysInfo::machineUniqueId();
    return value.isEmpty() ? QString() : QString::fromLatin1(QCryptographicHash::hash(value, QCryptographicHash::Sha256).toHex());
}
QString localDirectory() {
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/uFlipper/local-mcp/" + machineIdentity();
}
QJsonObject readPrivateJson(const QString &path) {
    struct stat info{};
    const auto native = QFile::encodeName(path);
    if (::lstat(native.constData(), &info) || !S_ISREG(info.st_mode) || info.st_uid != ::geteuid() ||
        (info.st_mode & 0077) || info.st_size > 4096) return {};
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return {};
    return QJsonDocument::fromJson(file.read(4097)).object();
}
LocalGateway::LocalGateway(QJsonArray tools, Protocol::Call call, std::function<void()> revoked,
                           QString directory, QString machine, QObject *parent)
    : QObject(parent), tools_(std::move(tools)), call_(std::move(call)), revoked_(std::move(revoked)),
      directory_(std::move(directory)), machine_(std::move(machine)) {
    server_.setSocketOptions(QLocalServer::UserAccessOption);
    connect(&server_, &QLocalServer::newConnection, this, &LocalGateway::accept);
    const auto value = readPrivateJson(directory_ + "/grant.json");
    if (value.value("schema") == 1 && value.value("machine") == machine_ && value.value("access") == "full")
        QTimer::singleShot(0, this, [this] { start(); });
}
LocalGateway::~LocalGateway() {
    server_.close(); closeClients();
    QFile::remove(directory_ + "/endpoint.json");
}
QStringList LocalGateway::clients() const {
    QStringList names;
    for (const auto &client : clients_) if (client->authenticated)
        names.append(client->protocol.clientName().isEmpty() ? tr("MCP-Client verbindet …") : client->protocol.clientName());
    return names;
}
QString LocalGateway::command() const {
    return QString::fromUtf8(QJsonDocument(QJsonObject{{"command", QCoreApplication::applicationFilePath()},
                       {"args", QJsonArray{"--mcp"}}}).toJson(QJsonDocument::Indented));
}
bool LocalGateway::save(const QString &name, const QJsonObject &value) {
    if (!QDir().mkpath(directory_) || !QFile::setPermissions(directory_, privateFile | QFileDevice::ExeOwner)) return false;
    QSaveFile file(directory_ + "/" + name);
    if (!file.open(QIODevice::WriteOnly) || !file.setPermissions(privateFile)) return false;
    const auto bytes = QJsonDocument(value).toJson(QJsonDocument::Compact);
    return file.write(bytes) == bytes.size() && file.commit();
}
bool LocalGateway::start() {
    if (enabled()) return true;
    if (machine_.isEmpty()) { error_ = tr("Keine sichere Zuordnung zu diesem Computer möglich."); emit changed(); return false; }
    token_ = nonce();
    const auto socket = QStringLiteral("/tmp/ufmcp-%1-%2").arg(::geteuid()).arg(nonce().left(24));
    if (!server_.listen(socket) || !save("endpoint.json", {{"schema", 1}, {"socket", server_.fullServerName()}, {"token", token_}})) {
        error_ = tr("Lokaler MCP-Zugang konnte nicht geöffnet werden.");
        server_.close(); QFile::remove(directory_ + "/endpoint.json"); token_.clear(); emit changed(); return false;
    }
    error_.clear(); emit changed(); return true;
}
void LocalGateway::grant() {
    if (enabled()) return;
    if (!start()) return;
    if (!save("grant.json", {{"schema", 1}, {"machine", machine_}, {"access", "full"}})) {
        revoke(); error_ = tr("MCP-Freigabe konnte nicht lokal gespeichert werden."); emit changed();
    }
}
void LocalGateway::closeClients() {
    const auto sockets = clients_.keys();
    clients_.clear();
    for (auto *socket : sockets) { socket->disconnect(this); socket->abort(); socket->deleteLater(); }
}
void LocalGateway::revoke() {
    server_.close(); token_.clear();
    QFile::remove(directory_ + "/endpoint.json");
    const bool removed = !QFile::exists(directory_ + "/grant.json") || QFile::remove(directory_ + "/grant.json");
    closeClients(); revoked_();
    error_ = removed ? QString() : tr("Die lokale Freigabedatei konnte nicht gelöscht werden. Vor einem Neustart prüfen.");
    emit changed();
}
void LocalGateway::accept() {
    while (auto *socket = server_.nextPendingConnection()) {
        if (clients_.size() >= 8) { socket->abort(); socket->deleteLater(); continue; }
        socket->setReadBufferSize(MaxMessage + 1);
        clients_.insert(socket, std::make_shared<Client>(tools_, call_));
        connect(socket, &QLocalSocket::readyRead, this, [this, socket] { receive(socket); });
        connect(socket, &QLocalSocket::disconnected, this, [this, socket] {
            clients_.remove(socket); socket->deleteLater(); emit changed();
        });
        QTimer::singleShot(5000, socket, [this, socket] {
            if (clients_.contains(socket) && !clients_.value(socket)->authenticated) socket->abort();
        });
    }
}
void LocalGateway::receive(QLocalSocket *socket) {
    const auto client = clients_.value(socket);
    if (!client) return;
    client->input += socket->readAll();
    if (client->input.size() > MaxBuffered) { socket->abort(); return; }
    // A bounded frame and bounded output queue also protect the GUI event loop.
    for (int count = 0; count < 16; ++count) {
        const auto newline = client->input.indexOf('\n');
        if (newline < 0) {
            if (client->input.size() > MaxMessage) socket->abort();
            return;
        }
        if (newline > MaxMessage || socket->bytesToWrite() > MaxBuffered) { socket->abort(); return; }
        const auto line = client->input.left(newline);
        client->input.remove(0, newline + 1);
        if (!client->authenticated) {
            const auto auth = QJsonDocument::fromJson(line).object();
            if (token_.isEmpty() || auth.value("token").toString() != token_) { socket->abort(); return; }
            client->authenticated = true;
            socket->write("{\"authenticated\":true}\n"); emit changed();
        } else {
            const auto response = client->protocol.receive(line);
            if (!response.isEmpty()) {
                const auto bytes = QJsonDocument(response).toJson(QJsonDocument::Compact) + '\n';
                if (bytes.size() > MaxBuffered || socket->bytesToWrite() + bytes.size() > MaxBuffered) { socket->abort(); return; }
                socket->write(bytes);
            }
            emit changed();
        }
    }
    if (client->input.contains('\n')) QTimer::singleShot(0, socket, [this, socket] { receive(socket); });
}
}
