#include "protocol.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <cmath>
#include <exception>
namespace Mcp {
namespace {
QJsonArray versions() { return {"2026-07-28", "2025-11-25"}; }
QJsonObject identity() { return {{"name", "uFlipper"}, {"version", "0.1.0"}}; }
QJsonObject capabilities() { return {{"tools", QJsonObject{}}}; }
bool validId(const QJsonValue &id) {
    return id.isString() || (id.isDouble() && std::isfinite(id.toDouble()) &&
                            std::floor(id.toDouble()) == id.toDouble() && std::abs(id.toDouble()) <= 9007199254740991.0);
}
bool validArguments(const QJsonObject &schema, const QJsonObject &args) {
    const auto properties = schema.value("properties").toObject();
    for (const auto &key : schema.value("required").toArray()) if (!args.contains(key.toString())) return false;
    for (auto it = args.begin(); it != args.end(); ++it) {
        if (!properties.contains(it.key())) return false;
        const auto rule = properties.value(it.key()).toObject();
        const auto type = rule.value("type").toString();
        if (type == "string" && (!it->isString() || it->toString().size() > 2048)) return false;
        if (type == "integer" && (!it->isDouble() || std::floor(it->toDouble()) != it->toDouble())) return false;
        if (rule.contains("enum") && !rule.value("enum").toArray().contains(*it)) return false;
        if (rule.contains("minimum") && it->toDouble() < rule.value("minimum").toDouble()) return false;
        if (rule.contains("maximum") && it->toDouble() > rule.value("maximum").toDouble()) return false;
    }
    return true;
}
}
Protocol::Protocol(QJsonArray tools, Call call) : tools_(std::move(tools)), call_(std::move(call)) {}
QJsonObject Protocol::tool(const QString &name, const QString &description, QJsonObject properties,
                          QJsonArray required, bool readOnly) {
    return {{"name", name}, {"description", description},
            {"inputSchema", QJsonObject{{"type", "object"}, {"properties", properties},
                                       {"required", required}, {"additionalProperties", false}}},
            {"annotations", QJsonObject{{"readOnlyHint", readOnly}, {"destructiveHint", !readOnly},
                                        {"openWorldHint", !readOnly}}}};
}
QJsonObject Protocol::error(const QJsonValue &id, int code, const QString &message, QJsonObject data) {
    QJsonObject value{{"code", code}, {"message", message}};
    if (!data.isEmpty()) value.insert("data", data);
    return {{"jsonrpc", "2.0"}, {"id", id.isUndefined() ? QJsonValue(QJsonValue::Null) : id}, {"error", value}};
}
QJsonObject Protocol::result(const QJsonValue &id, QJsonObject data, bool modern) const {
    if (modern) {
        data.insert("resultType", "complete");
        data.insert("_meta", QJsonObject{{"io.modelcontextprotocol/serverInfo", identity()}});
    }
    return {{"jsonrpc", "2.0"}, {"id", id}, {"result", data}};
}
QJsonObject Protocol::receive(const QByteArray &line) {
    QJsonParseError parse;
    const auto doc = QJsonDocument::fromJson(line, &parse);
    if (parse.error != QJsonParseError::NoError) return error({}, -32700, "Invalid JSON");
    if (!doc.isObject()) return error({}, -32600, "A single JSON-RPC object is required");
    const auto msg = doc.object();
    const auto id = msg.value("id");
    const bool notification = !msg.contains("id");
    if (msg.value("jsonrpc") != "2.0" || !msg.value("method").isString() ||
        (!notification && !validId(id))) return error({}, -32600, "Invalid JSON-RPC request");
    const auto method = msg.value("method").toString();
    if (msg.contains("params") && !msg.value("params").isObject())
        return notification ? QJsonObject{} : error(id, -32602, "params must be an object");
    const auto params = msg.value("params").toObject();
    // Notifications never execute tools. Calls return job handles immediately;
    // canceling a completed request does not cancel the separately identified job.
    if (notification) {
        if (method == "notifications/initialized" && initialized_) ready_ = true;
        return {};
    }
    if (method == "initialize") {
        if (initialized_) return error(id, -32600, "Already initialized");
        if (!params.value("protocolVersion").isString() || !params.value("capabilities").isObject() ||
            !params.value("clientInfo").isObject()) return error(id, -32602, "Invalid initialize parameters");
        // A legacy handshake can only negotiate a legacy revision. Modern
        // clients use per-request metadata rather than an initialize handshake.
        const auto client = params.value("clientInfo").toObject();
        if (!client.value("name").isString() || !client.value("version").isString())
            return error(id, -32602, "Invalid clientInfo");
        initialized_ = true; client_ = client.value("name").toString().left(128);
        return result(id, {{"protocolVersion", "2025-11-25"}, {"capabilities", capabilities()},
                           {"serverInfo", identity()},
                           {"instructions", "Use job handles for status/stop. Hardware conflicts never stop an existing session automatically."}}, false);
    }
    const auto meta = params.value("_meta").toObject();
    const bool modern = meta.contains("io.modelcontextprotocol/protocolVersion");
    if (modern) {
        if (meta.value("io.modelcontextprotocol/protocolVersion") != "2026-07-28")
            return error(id, -32022, "Unsupported protocol version", {{"supported", versions()},
                         {"requested", meta.value("io.modelcontextprotocol/protocolVersion")}});
        if (!meta.value("io.modelcontextprotocol/clientCapabilities").isObject())
            return error(id, -32602, "Per-request clientCapabilities are required");
        const auto client = meta.value("io.modelcontextprotocol/clientInfo").toObject();
        if (meta.contains("io.modelcontextprotocol/clientInfo") &&
            (!meta.value("io.modelcontextprotocol/clientInfo").isObject() ||
             !client.value("name").isString() || !client.value("version").isString()))
            return error(id, -32602, "Invalid per-request clientInfo");
        client_ = client.value("name").toString("Unnamed MCP client").left(128);
    } else if (method != "ping" && !ready_) {
        return error(id, -32002, "Initialize the legacy client or include modern per-request metadata");
    }
    if (method == "ping" && !modern) return result(id, {}, false);
    if (method == "server/discover" && modern)
        return result(id, {{"supportedVersions", versions()}, {"capabilities", capabilities()},
                           {"ttlMs", 0}, {"cacheScope", "private"}}, true);
    if (method == "tools/list") {
        if (params.contains("cursor")) return error(id, -32602, "This tool list has no further pages");
        QJsonObject data{{"tools", tools_}};
        if (modern) { data.insert("ttlMs", 0); data.insert("cacheScope", "private"); }
        return result(id, data, modern);
    }
    if (method != "tools/call") return error(id, -32601, "Method not found");
    if (!params.value("name").isString() || (params.contains("arguments") && !params.value("arguments").isObject()))
        return error(id, -32602, "Invalid tool call");
    QJsonObject selected;
    for (const auto &tool : tools_) if (tool.toObject().value("name") == params.value("name")) selected = tool.toObject();
    if (selected.isEmpty()) return error(id, -32602, "Unknown tool");
    const auto args = params.value("arguments").toObject();
    if (!validArguments(selected.value("inputSchema").toObject(), args))
        return error(id, -32602, "Arguments do not match the tool schema");
    ToolReply reply;
    try { reply = call_(params.value("name").toString(), args); }
    catch (const std::exception &) { reply = {{{"code", "internal_error"}, {"message", "The action failed. Inspect job status before retrying."}}, true}; }
    return result(id, {{"content", QJsonArray{QJsonObject{{"type", "text"},
                 {"text", QString::fromUtf8(QJsonDocument(reply.data).toJson(QJsonDocument::Compact))}}}},
                      {"structuredContent", reply.data}, {"isError", reply.error}}, modern);
}
}
