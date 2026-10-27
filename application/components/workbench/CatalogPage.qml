import QtQuick 2.15
import QtQuick.Layouts 1.15
import QFlipper 1.0
WorkbenchSplit {
    id: catalog
    objectName: "catalogPage"
    showDetails: false
    WorkbenchTitle { title: qsTr("Backups"); subtitle: qsTr("Lokale Sicherungen und ihr dokumentierter Umfang.") }
    WorkbenchHint { text: qsTr("Vollständige Gerätesicherung und Wiederherstellung sind noch nicht angeschlossen. Firmwareänderungen und Restore bleiben bis zu einem geprüften Vollbackup angehalten.") }
    WorkbenchButton { text: qsTr("Datenordner öffnen"); onClicked: Qt.openUrlExternally(Backend.workbench.storageUrl) }
    WorkbenchLabel { text: Backend.workbench.error || (Backend.workbench.backups.length ? qsTr("Lokaler Katalog") : qsTr("Noch keine lokalen Sicherungen")); font.pixelSize: 21; Layout.fillWidth: true }
    Repeater {
        model: Backend.workbench.backups
        delegate: ColumnLayout {
            required property var modelData
            Layout.fillWidth: true; spacing: 12
            RowLayout {
                Layout.fillWidth: true
                WorkbenchFact { label: modelData.scope === "test-subset" ? qsTr("Testsicherung · nur Testdateien") : qsTr("Gerätesicherung"); value: Qt.formatDateTime(new Date(modelData.started_ms), "dd.MM.yyyy hh:mm") }
                WorkbenchLabel { text: modelData.state === "verified" ? qsTr("Katalogstatus: geprüft") : qsTr("Unterbrochen / unvollständig"); font.pixelSize: 14; opacity: 0.7 }
                WorkbenchButton { text: qsTr("Öffnen"); onClicked: Qt.openUrlExternally(Backend.workbench.backupUrl(modelData.id)) }
            }
            WorkbenchLabel { text: modelData.id; font.family: "Share Tech Mono"; font.pixelSize: 13; Layout.fillWidth: true; opacity: 0.65 }
            WorkbenchRule {}
        }
    }
}
