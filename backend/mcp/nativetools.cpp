#include "nativetools.h"
#include "applicationbackend.h"
#include "workbench/nfccontroller.h"
#include "workbench/backupcontroller.h"
#include "workbench/workbenchdata.h"
#include "workbench/devicetelemetry.h"
#include "flipperzero/filemanager.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QUrl>
namespace Mcp {
namespace {
QJsonObject stringType() { return {{"type", "string"}}; }
ToolReply failure(const QString &code, const QString &message) { return {{{"code", code}, {"message", message}}, true}; }
QJsonObject nfcStatus(ApplicationBackend *backend) {
    auto *nfc = backend->nfc();
    return {{"jobId", nfc->jobId()}, {"origin", nfc->origin()}, {"state", nfc->state()},
            {"active", nfc->active()}, {"blocked", nfc->blocked()}, {"mode", nfc->mode()},
            {"recordingId", nfc->recordingId()}, {"stopConfirmed", nfc->stopConfirmed()},
            {"recordingVerified", nfc->recordingVerified()}, {"error", nfc->error()}};
}
ToolReply readArtifact(ApplicationBackend *backend, const QJsonObject &args) {
    const auto uri = args.value("uri").toString();
    const auto match = QRegularExpression("^uflipper://recording/([a-f0-9]{32})/(manifest|events)$").match(uri);
    if (!match.hasMatch()) return failure("invalid_artifact", "Use a recording URI returned by recordings_list.");
    const auto id = match.captured(1);
    bool finalized = false;
    for (const auto &value : backend->workbench()->recordings()) {
        const auto item = value.toMap();
        if (item.value("id") == id && (item.value("state") == "completed" || item.value("state") == "interrupted")) finalized = true;
    }
    if (!finalized) return failure("not_finalized", "The recording is missing or is still being written.");
    const auto folder = backend->workbench()->recordingUrl(id).toLocalFile();
    const auto filename = match.captured(2) == "manifest" ? "manifest.json" : "events.bin";
    if (QFileInfo(folder).isSymLink() || QFileInfo(folder + "/" + filename).isSymLink())
        return failure("invalid_artifact", "Symbolic links are not recording artifacts.");
    QFile file(folder + "/" + filename);
    const auto offset = qint64(args.value("offset").toDouble(0));
    const auto limit = args.value("limit").toInt(16384);
    if (!file.open(QIODevice::ReadOnly) || offset > file.size() || !file.seek(offset))
        return failure("artifact_unavailable", "The recording chunk cannot be read.");
    const auto bytes = file.read(limit);
    if (file.error() != QFileDevice::NoError) return failure("artifact_read_failed", "Reading the recording failed.");
    return {{{"uri", uri}, {"offset", double(offset)}, {"nextOffset", double(offset + bytes.size())},
             {"totalBytes", double(file.size())}, {"eof", file.atEnd()},
             {"encoding", "base64"}, {"data", QString::fromLatin1(bytes.toBase64())}}, false};
}
}
QJsonArray nativeTools() {
    return {
        Protocol::tool("capabilities", "Implemented device tools and explicit prerequisites/limitations."),
        Protocol::tool("device_status", "Current device, firmware, battery, power and storage snapshot including stale/unavailable flags."),
        Protocol::tool("jobs_list", "NFC jobs from the same session used by the native UI, plus current backup state."),
        Protocol::tool("nfc_start", "Start a persistently recorded read-only PPSE or Type 4A NDEF session using the already approved helper. Returns a job handle; poll jobs_list. Never installs helpers or stops another task.",
            {{"mode", QJsonObject{{"type", "string"}, {"enum", QJsonArray{"ppse", "ndef"}}}}}, {"mode"}, false),
        Protocol::tool("job_stop", "Explicitly stop the identified current NFC job. Poll jobs_list for confirmed stop; requesting stop alone is not confirmation.",
            {{"jobId", stringType()}}, {"jobId"}, false),
        Protocol::tool("recordings_list", "Latest 40 recordings from the same catalog used by the UI. Finalized recordings include bounded artifact references."),
        Protocol::tool("artifact_read", "Read at most 32 KiB of a finalized recording artifact. No arbitrary local filesystem access.",
            {{"uri", stringType()}, {"offset", QJsonObject{{"type", "integer"}, {"minimum", 0}, {"maximum", 9007199254740991.0}}},
             {"limit", QJsonObject{{"type", "integer"}, {"minimum", 1}, {"maximum", 32768}}}}, {"uri"}),
        Protocol::tool("backup_status", "Full-device backup prerequisite and latest catalog entries. Full backups remain blocked until a complete unfiltered inventory exists."),
        Protocol::tool("firmware_status", "Firmware update/install/restore prerequisites. No automatic update or bypass of the verified-full-backup requirement.")
    };
}
ToolReply nativeCall(ApplicationBackend *backend, const QString &name, const QJsonObject &args) {
    if (name == "capabilities") return {{{"implemented", QJsonArray{"device_status", "nfc.ppse", "nfc.type4a.ndef", "jobs", "recordings", "artifact_chunks"}},
        {"nfcCanStart", backend->nfc()->canStart()},
        {"helper", QJsonObject{{"version", "0.1.0"}, {"installation", "Separate explicit approval required for every version; MCP does not install helpers"}}},
        {"notYetImplemented", QJsonArray{"MCP device files", "IR", "Sub-GHz", "LF RFID", "iButton", "GPIO", "UART/I2C/SPI", "USB/Bluetooth", "apps/scripts", "firmware library", "cloud"}},
        {"fullBackupBlocked", true}, {"backupLimitation", backend->backup()->limitation()},
        {"systemReader", "Experimental native UI only; component installation and independent card proof still outstanding"}}, false};
    if (name == "device_status") return {QJsonObject::fromVariantMap(backend->telemetry()->snapshot()), false};
    if (name == "jobs_list") return {{{"nfc", nfcStatus(backend)}, {"nfcHistory", QJsonArray::fromVariantList(backend->nfc()->jobs())},
        {"backup", QJsonObject{{"state", backend->backup()->state()}, {"active", backend->backup()->active()},
                                {"blocked", backend->backup()->blocked()}}}}, false};
    if (name == "nfc_start") {
        // Preserve the session already running, irrespective of who started it.
        if (backend->nfc()->blocked() || backend->backup()->blocked() || backend->fileManager()->operationPending())
            return failure("hardware_conflict", "A device task is active or its stop is unconfirmed. Inspect jobs_list; explicitly stop the identified task before retrying.");
        if (!backend->nfc()->canStart()) return failure("device_not_ready", "Connect the Flipper and wait for the shared native connection to become ready.");
        backend->nfc()->startFromMcp(args.value("mode").toString());
        if (backend->nfc()->jobId().isEmpty() || backend->nfc()->origin() != "mcp")
            return failure("start_failed", backend->nfc()->error());
        return {nfcStatus(backend), !backend->nfc()->active() && !backend->nfc()->error().isEmpty()};
    }
    if (name == "job_stop") {
        if (args.value("jobId").toString().isEmpty() || args.value("jobId").toString() != backend->nfc()->jobId())
            return failure("unknown_job", "The job handle does not identify the current NFC session.");
        backend->nfc()->stop(); return {nfcStatus(backend), false};
    }
    if (name == "recordings_list") {
        backend->workbench()->refresh();
        QJsonArray items;
        for (const auto &value : backend->workbench()->recordings()) {
            auto item = QJsonObject::fromVariantMap(value.toMap());
            const auto id = item.value("id").toString();
            if (item.value("state") == "completed" || item.value("state") == "interrupted")
                item.insert("artifacts", QJsonArray{"uflipper://recording/" + id + "/manifest", "uflipper://recording/" + id + "/events"});
            items.append(item);
        }
        if (!backend->workbench()->error().isEmpty()) return failure("catalog_unavailable", backend->workbench()->error());
        return {{{"recordings", items}, {"limit", 40}}, false};
    }
    if (name == "artifact_read") return readArtifact(backend, args);
    if (name == "backup_status") return {{{"canStart", backend->backup()->canStart()},
        {"limitation", backend->backup()->limitation()}, {"state", backend->backup()->state()},
        {"backups", QJsonArray::fromVariantList(backend->workbench()->backups())}}, false};
    if (name == "firmware_status") return {{{"actionsAvailable", false},
        {"reason", "A verified automatic full-device backup is required before update, firmware change or restore."},
        {"backupLimitation", backend->backup()->limitation()}, {"library", "not yet implemented"}}, false};
    return failure("unknown_tool", "Tool is not implemented.");
}
}
