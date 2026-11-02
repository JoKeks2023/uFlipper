#include "mcp/protocol.h"
#include "mcp/localgateway.h"
#include "mcp/stdiorelay.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QSocketNotifier>
#include <QTemporaryDir>
#include <cstdio>
#include <stdexcept>
#include <unistd.h>
using namespace Mcp;
namespace {
void require(bool ok, const char *message) { if (!ok) throw std::runtime_error(message); }
QJsonObject modernMeta() { return {{"io.modelcontextprotocol/protocolVersion", "2026-07-28"},
                                   {"io.modelcontextprotocol/clientCapabilities", QJsonObject{}}}; }
QJsonObject message(QString method, QJsonObject params = {}) { return {{"jsonrpc", "2.0"}, {"id", 1}, {"method", method}, {"params", params}}; }
QJsonObject receive(Protocol &p, QJsonObject msg) { return p.receive(QJsonDocument(msg).toJson(QJsonDocument::Compact)); }
void contracts() {
    int calls = 0;
    const QJsonArray tools{Protocol::tool("fixture", "Synthetic protocol fixture", {{"limit", QJsonObject{{"type", "integer"}, {"minimum", 1}, {"maximum", 32768}}}}, {}, false)};
    Protocol p(tools, [&](const QString &, const QJsonObject &) { ++calls; return ToolReply{{{"fixture", true}}, false}; });
    require(p.receive("{").value("error").toObject().value("code") == -32700, "Malformed JSON accepted");
    require(p.receive("[]").contains("error"), "Batch accepted");
    require(receive(p, message("tools/list")).contains("error"), "Ungated legacy request accepted");
    auto bad = modernMeta(); bad.insert("io.modelcontextprotocol/protocolVersion", "1900-01-01");
    const auto unsupported = receive(p, message("tools/list", {{"_meta", bad}})).value("error").toObject();
    require(unsupported.value("code") == -32022 && unsupported.value("data").toObject().value("supported").toArray().size() == 2, "Unsupported modern version not diagnosed");
    const auto discovery = receive(p, message("server/discover", {{"_meta", modernMeta()}})).value("result").toObject();
    require(discovery.value("resultType") == "complete" && discovery.contains("supportedVersions") && discovery.contains("ttlMs"), "Modern discovery invalid");
    require(receive(p, message("tools/call", {{"_meta", modernMeta()}, {"name", "fixture"}, {"arguments", QJsonObject{{"limit", 32769}}}})).contains("error") && calls == 0, "Oversized chunk dispatched");
    require(receive(p, message("tools/call", {{"_meta", modernMeta()}, {"name", "fixture"}, {"arguments", QJsonObject{{"limit", 1.5}}}})).contains("error"), "Fractional byte count dispatched");
    require(receive(p, message("tools/call", {{"_meta", modernMeta()}, {"name", "fixture"}, {"arguments", QJsonObject{{"path", "/etc/passwd"}}}})).contains("error"), "Unknown argument dispatched");
    auto notification = message("tools/call", {{"name", "fixture"}}); notification.remove("id");
    require(receive(p, notification).isEmpty() && calls == 0, "Notification executed device tool");
    auto noCaps = modernMeta(); noCaps.remove("io.modelcontextprotocol/clientCapabilities");
    require(receive(p, message("tools/list", {{"_meta", noCaps}})).contains("error"), "Capabilities inferred across modern requests");
    const auto info = QJsonObject{{"name", "independent-fixture"}, {"version", "1"}};
    const auto init = receive(p, message("initialize", {{"protocolVersion", "2025-11-25"}, {"capabilities", QJsonObject{}}, {"clientInfo", info}}));
    require(init.value("result").toObject().value("protocolVersion") == "2025-11-25", "Legacy version changed");
    require(receive(p, message("tools/list")).contains("error"), "Legacy tools accepted before initialized");
    auto initialized = message("notifications/initialized"); initialized.remove("id"); receive(p, initialized);
    require(receive(p, message("tools/list")).value("result").toObject().value("tools").toArray().size() == 1, "Legacy tool list failed");
    require(!receive(p, message("tools/call", {{"name", "fixture"}})).value("result").toObject().value("isError").toBool() && calls == 1, "Legacy tool dispatch failed");
    require(receive(p, message("tools/list", {{"_meta", modernMeta()}})).value("result").toObject().value("resultType") == "complete", "Dual-era support lost");
    QTemporaryDir directory;
    bool revoked = false;
    LocalGateway gateway(tools, [](const QString &, const QJsonObject &) { return ToolReply{}; }, [&] { revoked = true; }, directory.path(), "fixture-machine");
    require(!gateway.enabled() && readPrivateJson(directory.path() + "/endpoint.json").isEmpty(), "Fresh profile is granted");
    gateway.grant(); require(gateway.enabled(), "Explicit fixture grant failed");
    require(readPrivateJson(directory.path() + "/grant.json").value("machine") == "fixture-machine", "Grant is not machine bound");
    gateway.revoke(); require(revoked && !gateway.enabled() && readPrivateJson(directory.path() + "/grant.json").isEmpty(), "Revoke retained grant");
    std::fprintf(stderr, "MCP protocol and grant contracts passed\n");
}
}
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    const auto args = app.arguments();
    try {
        if (args.size() == 3 && args[1] == "--relay") return runStdioRelay(args[2]);
        if (args.size() >= 3 && args[1] == "--fixture-server") {
            // No production backend, serial connection or hardware in this host.
            LocalGateway gateway({Protocol::tool("fixture_status", "Synthetic status, no hardware")},
                [](const QString &, const QJsonObject &) { return ToolReply{{{"fixture", true}}, false}; },
                [] {}, args[2], args.value(3, "fixture-machine"));
            QSocketNotifier input(STDIN_FILENO, QSocketNotifier::Read);
            QObject::connect(&input, &QSocketNotifier::activated, &app, [&] {
                char bytes[64]; const auto count = ::read(STDIN_FILENO, bytes, sizeof(bytes));
                if (count <= 0) { app.quit(); return; }
                const QByteArray command(bytes, count);
                if (command.startsWith("grant")) gateway.grant();
                else if (command.startsWith("revoke")) gateway.revoke();
                else if (command.startsWith("quit")) { app.quit(); return; }
                std::printf("%s\n", gateway.enabled() ? "enabled" : "disabled"); std::fflush(stdout);
            });
            QTimer::singleShot(0, &app, [&] { std::printf("ready:%s\n", gateway.enabled() ? "enabled" : "disabled"); std::fflush(stdout); });
            return app.exec();
        }
        contracts(); return 0;
    } catch (const std::exception &error) { std::fprintf(stderr, "%s\n", error.what()); return 1; }
}
