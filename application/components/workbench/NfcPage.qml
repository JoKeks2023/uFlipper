import QtQuick 2.15
import QtQuick.Layouts 1.15
import QFlipper 1.0
WorkbenchSplit {
    id: page
    objectName: "nfcPage"
    readonly property var session: Backend.nfc
    readonly property var saved: Backend.workbench.recordings.filter(function(item) { return item.kind === "nfc.iso14443-4a.ppse" })
    readonly property var labels: ({idle:qsTr("Bereit"), starting:qsTr("Helfer wird geprüft und gestartet …"), searching:qsTr("Warte auf Karte"), reading:qsTr("Karte wird gelesen …"), stopping:qsTr("Wird gestoppt …"), completed:qsTr("Lesetest abgeschlossen"), stopped:qsTr("Gestoppt"), interrupted:qsTr("Unterbrochen"), uncertain:qsTr("Stop nicht bestätigt"), saved:qsTr("Gespeicherte Sitzung")})
    WorkbenchTitle { title: qsTr("NFC"); subtitle: qsTr("ISO-DEP-Lesetest · Kartenstatus, Antworten und gespeicherte Sitzungen.") }
    WorkbenchHint {
        visible: Preferences.easyMode
        text: qsTr("Lesetest starten und die Karte an die Rückseite des Flippers halten. Für Apple Pay die Karte am iPhone auswählen und bestätigen. Nach zwei Verzeichnisabfragen endet der Test automatisch.")
    }
    RowLayout {
        Layout.fillWidth: true; spacing: 16
        WorkbenchFact { Layout.fillWidth: true; label: qsTr("Sitzung"); value: page.labels[page.session.state] || page.session.state }
        WorkbenchButton { objectName: "nfc-start"; text: qsTr("Lesetest starten"); enabled: page.session.canStart; onClicked: page.session.start() }
        WorkbenchButton { objectName: "nfc-stop"; text: qsTr("Stoppen"); enabled: page.session.active && page.session.state !== "stopping"; onClicked: page.session.stop() }
    }
    WorkbenchHint { visible: page.session.error.length > 0; text: page.session.error }
    WorkbenchRule {}
    WorkbenchLabel { text: qsTr("Karte"); font.pixelSize: 21 }
    RowLayout {
        Layout.fillWidth: true; spacing: 24
        WorkbenchFact { Layout.fillWidth: true; label: qsTr("Protokoll"); value: page.session.card.protocol || qsTr("Noch keine Karte erkannt") }
        WorkbenchFact { Layout.fillWidth: true; label: qsTr("UID dieser Sitzung"); value: page.session.card.uid || "—" }
    }
    WorkbenchLabel { text: qsTr("Antworten"); font.pixelSize: 21 }
    WorkbenchLabel { visible: page.session.responses.length === 0; text: qsTr("Noch keine Kommandoantworten"); opacity: 0.65 }
    Repeater {
        model: page.session.responses
        ColumnLayout {
            required property var modelData
            Layout.fillWidth: true; spacing: 8
            RowLayout {
                Layout.fillWidth: true; spacing: 16
                WorkbenchLabel { text: modelData.command; Layout.fillWidth: true }
                WorkbenchLabel { text: modelData.bytes + qsTr(" Bytes · Status ") + modelData.statusWord }
            }
            WorkbenchLabel { Layout.fillWidth: true; text: modelData.hex; wrapMode: Text.Wrap; font.pixelSize: 14; opacity: 0.75 }
            WorkbenchRule {}
        }
    }
    WorkbenchLabel { text: qsTr("Gespeicherte Sitzungen"); font.pixelSize: 21 }
    WorkbenchLabel { visible: page.saved.length === 0; text: qsTr("Nach dem ersten Start erscheint hier die automatisch gespeicherte Sitzung."); Layout.fillWidth: true; wrapMode: Text.Wrap; opacity: 0.65 }
    Repeater {
        model: page.saved
        RowLayout {
            required property var modelData
            Layout.fillWidth: true; spacing: 16
            WorkbenchFact { Layout.fillWidth: true; label: Qt.formatDateTime(new Date(modelData.started_ms), "dd.MM.yyyy hh:mm:ss"); value: modelData.state + " · " + modelData.bytes + " Bytes" }
            WorkbenchButton { objectName: "nfc-open-" + modelData.id; text: qsTr("Öffnen"); enabled: !page.session.blocked; onClicked: page.session.openRecording(modelData.id) }
            WorkbenchButton { text: qsTr("Originaldaten"); onClicked: Qt.openUrlExternally(Backend.workbench.recordingUrl(modelData.id)) }
        }
    }
    details: [
        WorkbenchLabel { text: qsTr("Lesetest"); font.pixelSize: 21 },
        WorkbenchLabel { Layout.fillWidth: true; text: qsTr("Modus"); font.pixelSize: 14; opacity: 0.65 },
        WorkbenchComboBox { Layout.fillWidth: true; model: [qsTr("EMV-Verzeichnis (PPSE)")]; enabled: false },
        WorkbenchLabel { Layout.fillWidth: true; text: qsTr("ISO14443-4A. Zwei identische Verzeichnisabfragen; keine Zahlung. Allgemeines Karten-Auslesen, NDEF und der Mac-Systemreader folgen separat."); wrapMode: Text.Wrap; font.pixelSize: 14; opacity: 0.75 },
        WorkbenchRule {},
        WorkbenchFact { label: qsTr("Aufzeichnung"); value: page.session.active ? qsTr("Läuft · Originaldaten werden gespeichert") : page.session.recordingVerified ? qsTr("Gespeichert und geprüft") : qsTr("Automatisch vor jedem Start") },
        WorkbenchFact { label: qsTr("Geräte-Stop"); value: page.session.stopConfirmed ? qsTr("Bestätigt") : page.session.active ? qsTr("Sitzung läuft") : "—" },
        WorkbenchButton { text: qsTr("Sitzungsordner öffnen"); Layout.fillWidth: true; enabled: page.session.recordingId.length > 0; onClicked: Qt.openUrlExternally(page.session.recordingUrl()) },
        WorkbenchRule {},
        WorkbenchLabel { Layout.fillWidth: true; text: qsTr("Benötigt den bereits freigegebenen NFC-Helfer 0.1.0 für Momentum mntm-012. Vor dem Start wird seine Prüfsumme kontrolliert."); wrapMode: Text.Wrap; font.pixelSize: 14; opacity: 0.65 }
    ]
}
