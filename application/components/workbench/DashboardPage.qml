import QtQuick 2.15
import QtQuick.Layouts 1.15
import Theme 1.0
import QFlipper 1.0
WorkbenchSplit {
    id: dashboard
    objectName: "dashboardPage"
    signal navigate(string page)
    readonly property var snapshot: Backend.telemetry.snapshot
    readonly property var device: snapshot.device || ({})
    readonly property var power: snapshot.power || ({})
    // Pinned Momentum source: storage_processing.c redirects /int to EXT_PATH(...).
    // Keep unknown firmware as a logical RPC area; never infer physical flash capacity.
    readonly property bool internalOnSd: device.firmwareVersion === "mntm-012" &&
                                         (device.firmwareCommit || "").indexOf("e1784e74") === 0
    function metric(key, divisor, unit) {
        const m = (power.metrics || ({}))[key]
        return m && m.available ? Number(m.value / (divisor || 1)).toLocaleString(Qt.locale(), 'f', divisor === 1000 ? 3 : 0) + (unit || "") : "—"
    }
    function chargeState() {
        const m = (power.metrics || ({}))["charge.state"]
        if (!m || !m.available) return qsTr("Nicht verfügbar")
        if (m.value === "charging") return qsTr("Lädt")
        if (m.value === "charged") return qsTr("Geladen")
        if (m.value === "discharging") return qsTr("Entlädt")
        return m.value
    }
    WorkbenchTitle { title: qsTr("Übersicht"); subtitle: qsTr("Gerät, Versorgung und laufende Arbeit auf einen Blick.") }
    Rectangle {
        Layout.fillWidth: true; implicitHeight: 120
        radius: 6; color: "#120900"; border.color: Theme.color.mediumorange2
        RowLayout {
            anchors.fill: parent; anchors.margins: 16; spacing: 16
            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: false; Layout.alignment: Qt.AlignTop; spacing: 8
                WorkbenchLabel { text: qsTr("FLIPPER ZERO"); font.pixelSize: 13; opacity: 0.7 }
                WorkbenchLabel { text: dashboard.device.firmwareVersion || qsTr("Kein Gerät verbunden"); font.pixelSize: 23; Layout.fillWidth: true }
                WorkbenchLabel { text: dashboard.device.firmwareCommit ? qsTr("Firmware %1 · RPC %2.%3").arg(dashboard.device.firmwareCommit).arg(dashboard.device.rpcMajor).arg(dashboard.device.rpcMinor) : qsTr("Flipper per USB verbinden"); font.pixelSize: 14; opacity: 0.7; Layout.fillWidth: true }
            }
            Rectangle { width: 1; Layout.fillHeight: true; color: Theme.color.mediumorange2 }
            ColumnLayout {
                spacing: 8; Layout.preferredWidth: 120; Layout.fillHeight: false; Layout.alignment: Qt.AlignTop
                WorkbenchLabel { text: qsTr("AKKU"); font.pixelSize: 13; opacity: 0.7 }
                WorkbenchLabel { text: dashboard.metric("charge.level", 1, " %"); font.pixelSize: 32 }
                WorkbenchLabel { text: !dashboard.power.available ? qsTr("Keine Messung") : dashboard.power.stale ? qsTr("Veraltet") : dashboard.chargeState(); font.pixelSize: 14; opacity: 0.7 }
            }
        }
    }
    WorkbenchHint { visible: !!dashboard.power.error; text: dashboard.power.error || "" }
    WorkbenchLabel { text: qsTr("Speicher"); font.pixelSize: 21 }
    WorkbenchStorage { title: qsTr("SD-Karte"); info: (dashboard.snapshot.storage || ({}))["/ext"] || ({}) }
    WorkbenchStorage {
        title: qsTr("Interne Daten (/int)")
        info: (dashboard.snapshot.storage || ({}))["/int"] || ({})
        showCapacity: !dashboard.internalOnSd
        capacityText: dashboard.internalOnSd ? qsTr("Auf SD-Karte") : ""
        note: dashboard.internalOnSd ? qsTr("Momentum speichert diesen Bereich auf der SD-Karte. Die Kapazität ist oben enthalten.") :
                                      qsTr("Gemeldete Dateisystemkapazität für /int; kein Messwert der gesamten Geräte-Flashkapazität.")
    }
    WorkbenchRule {}
    WorkbenchLabel { text: qsTr("Aktuelle Arbeit"); font.pixelSize: 21 }
    WorkbenchFact { label: qsTr("Gerät"); value: Backend.deviceState ? Backend.deviceState.statusString || qsTr("Bereit") : qsTr("Wartet auf USB-Verbindung") }
    WorkbenchFact { visible: Backend.nfc.blocked; label: qsTr("NFC"); value: Backend.nfc.active ? qsTr("Lesesitzung aktiv · Aufnahme läuft") : qsTr("Stop nicht bestätigt · Flipper neu verbinden") }
    RowLayout {
        spacing: 8
        WorkbenchButton { visible: Backend.nfc.blocked; text: qsTr("NFC-Sitzung öffnen"); onClicked: dashboard.navigate("nfc") }
        WorkbenchButton { text: qsTr("Gerätebildschirm"); enabled: !!Backend.deviceState && !Backend.deviceState.isRecoveryMode; onClicked: dashboard.navigate("screen") }
        WorkbenchButton { text: qsTr("Dateien öffnen"); enabled: Backend.backendState === ApplicationBackend.Ready; onClicked: dashboard.navigate("library") }
    }
    WorkbenchRule {}
    WorkbenchLabel { text: qsTr("Letzte Aufnahmen"); font.pixelSize: 21 }
    WorkbenchLabel { visible: Backend.workbench.recordings.length === 0; text: Backend.workbench.error || qsTr("Noch keine lokalen Aufnahmen"); wrapMode: Text.Wrap; Layout.fillWidth: true; opacity: 0.65 }
    Repeater {
        model: Backend.workbench.recordings.slice(0, 4)
        delegate: ColumnLayout {
            required property var modelData
            Layout.fillWidth: true; spacing: 8
            RowLayout {
                spacing: 8
                Layout.fillWidth: true
                WorkbenchLabel { text: modelData.kind; Layout.fillWidth: true }
                WorkbenchLabel { text: modelData.state === "completed" ? qsTr("Abgeschlossen") : modelData.state === "interrupted" ? qsTr("Unterbrochen") : qsTr("Status im Katalog: läuft"); opacity: 0.65; font.pixelSize: 14 }
                WorkbenchButton { text: qsTr("Öffnen"); onClicked: Qt.openUrlExternally(Backend.workbench.recordingUrl(modelData.id)) }
            }
            WorkbenchRule {}
        }
    }
    details: [
        WorkbenchLabel { text: qsTr("Versorgung"); font.pixelSize: 21 },
        WorkbenchFact { label: qsTr("Ladezustand"); value: dashboard.chargeState() },
        WorkbenchRule {},
        WorkbenchFact { label: qsTr("Spannung"); value: dashboard.metric("battery.voltage", 1000, " V") },
        WorkbenchRule {},
        WorkbenchFact { label: qsTr("Strom"); value: dashboard.metric("battery.current", 1, " mA") },
        WorkbenchRule {},
        WorkbenchFact { label: qsTr("Temperatur"); value: dashboard.metric("battery.temp", 1, " °C") },
        WorkbenchRule {},
        WorkbenchFact { label: qsTr("Akku-Zustand"); value: dashboard.metric("battery.health", 1, " %") },
        WorkbenchFact { label: qsTr("Restkapazität"); value: dashboard.metric("capacity.remain", 1, " mAh") },
        WorkbenchFact { label: qsTr("Letzte Messung"); value: dashboard.power.observedAt ? Qt.formatDateTime(new Date(dashboard.power.observedAt), "hh:mm:ss") + (dashboard.power.stale ? qsTr(" · veraltet") : "") : qsTr("Noch keine Messung") },
        WorkbenchRule {},
        WorkbenchFact { label: qsTr("Vollbackup"); value: Backend.backup.active ? qsTr("Sicherung läuft") : Backend.backup.verified ? qsTr("Letzte Sitzung geprüft") : Backend.backup.limitation.length ? qsTr("Ungefiltertes Inventar fehlt") : qsTr("Lokalen Katalog öffnen") },
        WorkbenchButton { text: qsTr("Backups ansehen"); Layout.fillWidth: true; onClicked: dashboard.navigate("backups") },
        WorkbenchRule {},
        WorkbenchFact { label: qsTr("Synchronisierung"); value: qsTr("Noch nicht angeschlossen") }
    ]
}
