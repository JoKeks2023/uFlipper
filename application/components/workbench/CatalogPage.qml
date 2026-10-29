import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QFlipper 1.0
WorkbenchSplit {
    id: catalog
    objectName: "catalogPage"
    showDetails: false
    readonly property var session: Backend.backup
    readonly property var stateNames: ({idle: qsTr("Bereit"), inventory: qsTr("Speicher erfassen"), copying: qsTr("Dateien sichern"),
        "checking-inventory": qsTr("Vollständigkeit prüfen"), "checking-device": qsTr("Geräteinhalt vergleichen"),
        verifying: qsTr("Lokale Sicherung prüfen"), verified: qsTr("Vollbackup geprüft"), stopping: qsTr("Verbindung schließen"),
        cancelled: qsTr("Abgebrochen · unvollständig"), interrupted: qsTr("Unterbrochen · unvollständig"), uncertain: qsTr("Geräteverbindung ungeklärt")})
    function size(bytes) { return bytes >= 1048576 ? (bytes / 1048576).toFixed(1) + " MiB" : Math.round(bytes / 1024) + " KiB" }
    WorkbenchTitle { title: qsTr("Backups"); subtitle: qsTr("Interner Speicher und gesamte SD-Karte – mit Prüfung von Inhalt und Vollständigkeit.") }
    WorkbenchHint { text: qsTr("Die Sicherung liest alle über die Geräteschnittstelle erreichbaren Dateien einschließlich Apps, Ressourcen und leerer Ordner. Firmwareänderungen und Wiederherstellung bleiben bis zum geprüften automatischen Gesamtablauf angehalten.") }
    RowLayout {
        Layout.fillWidth: true; spacing: 16
        WorkbenchButton { objectName: "backup-start"; text: qsTr("Vollbackup erstellen"); primary: true; enabled: session.canStart && !Backend.nfc.blocked; onClicked: session.start() }
        WorkbenchButton { objectName: "backup-cancel"; text: qsTr("Abbrechen"); visible: session.active; enabled: session.canCancel; onClicked: session.cancel() }
        Item { Layout.fillWidth: true }
        WorkbenchButton { text: qsTr("Datenordner öffnen"); onClicked: Qt.openUrlExternally(Backend.workbench.storageUrl) }
    }
    WorkbenchLabel { Layout.fillWidth: true; wrapMode: Text.Wrap; visible: !session.canStart && !session.active; opacity: 0.7; text: Backend.nfc.blocked ? qsTr("Zuerst die laufende NFC-Sitzung beenden.") : qsTr("Flipper mit SD-Karte verbinden. Nach einem Abbruch den USB-Stecker kurz neu verbinden; laufende Dateiaktionen zunächst abschließen.") }
    WorkbenchRule {}
    WorkbenchFact { label: qsTr("Status"); value: catalog.stateNames[session.state] || session.state }
    WorkbenchFact { label: qsTr("Dateien"); value: session.filesDone + " / " + session.filesTotal }
    WorkbenchFact { label: qsTr("Daten"); value: catalog.size(session.bytesDone) + " / " + catalog.size(session.bytesTotal) }
    ProgressBar { Layout.fillWidth: true; visible: session.active; from: 0; to: Math.max(1, session.bytesTotal); value: session.bytesDone; indeterminate: session.state !== "copying" }
    WorkbenchLabel { Layout.fillWidth: true; text: session.currentPath; visible: text.length > 0; font.family: "Share Tech Mono"; font.pixelSize: 13; elide: Text.ElideMiddle }
    WorkbenchHint { visible: session.error.length > 0; text: session.error }
    WorkbenchLabel { Layout.fillWidth: true; wrapMode: Text.Wrap; visible: session.state === "verifying"; text: qsTr("Die gespeicherten Dateien werden von der Festplatte gelesen und geprüft. Bitte den Abschluss abwarten."); opacity: 0.7 }
    WorkbenchRule {}
    WorkbenchLabel { text: Backend.workbench.error || (Backend.workbench.backups.length ? qsTr("Lokaler Katalog") : qsTr("Noch keine lokalen Sicherungen")); font.pixelSize: 21; Layout.fillWidth: true }
    Repeater {
        model: Backend.workbench.backups
        delegate: ColumnLayout {
            required property var modelData
            Layout.fillWidth: true; spacing: 16
            RowLayout {
                spacing: 16; Layout.fillWidth: true
                WorkbenchFact { label: modelData.scope === "test-subset" ? qsTr("Testsicherung · nur Testdateien") : qsTr("Vollbackup"); value: Qt.formatDateTime(new Date(modelData.started_ms), "dd.MM.yyyy hh:mm") }
                WorkbenchLabel { text: modelData.state === "verified" ? qsTr("Prüfung abgeschlossen") : modelData.state === "writing" ? qsTr("In Arbeit") : qsTr("Unterbrochen / unvollständig"); font.pixelSize: 14; opacity: 0.7 }
                WorkbenchButton { objectName: "backup-open-" + modelData.id; text: qsTr("Öffnen"); onClicked: Qt.openUrlExternally(Backend.workbench.backupUrl(modelData.id)) }
            }
            WorkbenchLabel { text: modelData.reason || modelData.id; font.family: "Share Tech Mono"; font.pixelSize: 13; Layout.fillWidth: true; elide: Text.ElideMiddle; opacity: 0.65 }
            WorkbenchRule {}
        }
    }
}
