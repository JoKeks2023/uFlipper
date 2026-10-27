import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Theme 1.0
import QFlipper 1.0
WorkbenchSplit {
    id: settings
    objectName: "settingsPage"
    showDetails: false
    property int tab: 0
    WorkbenchTitle { title: qsTr("Einstellungen"); subtitle: qsTr("Allgemeines, Datenablage und Integrationen.") }
    RowLayout {
        Layout.fillWidth: true; spacing: 12
        Repeater {
            model: [qsTr("Allgemein"), qsTr("Speicher"), qsTr("Cloud"), qsTr("Integrationen"), qsTr("Über uFlipper")]
            WorkbenchButton {
                required property string modelData
                required property int index
                objectName: "settings-tab-" + index
                text: modelData; checked: settings.tab === index
                onClicked: settings.tab = index
            }
        }
    }
    WorkbenchRule {}
    ColumnLayout {
        visible: settings.tab === 0; Layout.fillWidth: true; spacing: 22
        WorkbenchLabel { text: qsTr("Darstellung und Arbeitsfläche"); font.pixelSize: 21 }
        RowLayout {
            Layout.fillWidth: true
            WorkbenchFact { label: qsTr("Design"); value: qsTr("qFlipper · Dunkel, Orange und Raster") }
            ComboBox { Layout.preferredWidth: 280; model: ["qFlipper"]; font.family: "Share Tech"; font.pixelSize: 17 }
        }
        WorkbenchRule {}
        RowLayout {
            Layout.fillWidth: true
            WorkbenchFact { label: qsTr("Arbeitsfläche"); value: qsTr("Workbench und separate Werkzeugfenster folgen.") }
            ComboBox {
                Layout.preferredWidth: 280
                model: [qsTr("Seitenleiste und Detailbereich"), qsTr("Workbench · in Arbeit"), qsTr("Werkzeugfenster · in Arbeit")]
                currentIndex: 0; font.family: "Share Tech"; font.pixelSize: 17
                delegate: ItemDelegate { required property int index; required property string modelData; width: parent ? parent.width : 280; text: modelData; enabled: index === 0; font.family: "Share Tech"; font.pixelSize: 17 }
                onActivated: currentIndex = 0
            }
        }
        WorkbenchRule {}
        RowLayout {
            Layout.fillWidth: true
            WorkbenchFact { label: qsTr("Easy Mode"); value: qsTr("Geführte Hinweise in denselben Werkzeugen") }
            ComboBox {
                objectName: "easy-mode-preference"
                Layout.preferredWidth: 280; model: [qsTr("Hinweise einblenden"), qsTr("Nur auf Wunsch einblenden")]
                currentIndex: Preferences.easyMode ? 0 : 1
                onActivated: Preferences.easyMode = currentIndex === 0
                font.family: "Share Tech"; font.pixelSize: 17
            }
        }
        WorkbenchRule {}
        RowLayout {
            Layout.fillWidth: true
            WorkbenchFact { label: qsTr("Dateiverwaltung"); value: qsTr("Versteckte Dateien anzeigen") }
            Switch { checked: Preferences.showHiddenFiles; onToggled: Preferences.showHiddenFiles = checked; Accessible.name: qsTr("Versteckte Dateien anzeigen") }
        }
        WorkbenchRule {}
        WorkbenchLabel { text: qsTr("Hintergrundbetrieb"); font.pixelSize: 21 }
        RowLayout {
            Layout.fillWidth: true
            WorkbenchFact { label: qsTr("Bei geschlossenem Fenster weiterarbeiten"); value: qsTr("Noch nicht verfügbar. Schließen beendet die Anwendung.") }
            Switch { checked: false; enabled: false; Accessible.name: qsTr("Hintergrundbetrieb noch nicht verfügbar") }
        }
    }
    ColumnLayout {
        visible: settings.tab === 1; Layout.fillWidth: true; spacing: 22
        WorkbenchLabel { text: qsTr("Lokale Ablage"); font.pixelSize: 21 }
        WorkbenchFact { label: qsTr("Aufnahmen und Backups"); value: decodeURIComponent(Backend.workbench.storageUrl.toString().replace("file://", "")) }
        WorkbenchButton { text: qsTr("Datenordner öffnen"); onClicked: Qt.openUrlExternally(Backend.workbench.storageUrl) }
        WorkbenchHint { text: qsTr("Aufnahmen behalten Originaldaten und Sitzungsinformationen. Testsicherungen sind von vollständigen Gerätesicherungen getrennt.") }
        WorkbenchRule {}
        WorkbenchFact { label: qsTr("Automatische Bereinigung"); value: qsTr("Aus. Es werden keine Aufnahmen oder Backups automatisch gelöscht.") }
        WorkbenchFact { label: qsTr("Papierkorb"); value: qsTr("Automatische Leerung aus. Synchronisierter Papierkorb wird noch angeschlossen.") }
        WorkbenchFact { label: qsTr("Aufbewahrung"); value: qsTr("Grenzen für Alter, Anzahl und Speicherverbrauch folgen mit der Datenverwaltung.") }
    }
    ColumnLayout {
        visible: settings.tab === 2; Layout.fillWidth: true; spacing: 20
        WorkbenchLabel { text: qsTr("Cloud-Verbindungen"); font.pixelSize: 21 }
        WorkbenchHint { text: qsTr("Die Cloud-Anbindungen sind noch nicht verfügbar. Es werden derzeit keine Daten hochgeladen.") }
        Repeater {
            model: ["Google Drive", "Dropbox", "OneDrive", "WebDAV", "S3"]
            RowLayout {
                required property string modelData
                Layout.fillWidth: true
                WorkbenchLabel { text: modelData; Layout.fillWidth: true }
                WorkbenchLabel { text: qsTr("Noch nicht angeschlossen"); opacity: 0.65 }
                WorkbenchButton { text: qsTr("Verbinden"); enabled: false }
            }
        }
        WorkbenchRule {}
        WorkbenchFact { label: qsTr("Verschlüsselung"); value: qsTr("Bei jeder neuen Verbindung ausdrücklich wählen. Schlüsseldatei/-code oder Masterpasswort.") }
        WorkbenchFact { label: qsTr("Konflikte"); value: qsTr("Nur den betroffenen Eintrag anhalten und eine Auswahl verlangen.") }
    }
    ColumnLayout {
        visible: settings.tab === 3; Layout.fillWidth: true; spacing: 22
        WorkbenchLabel { text: qsTr("MCP"); font.pixelSize: 21 }
        WorkbenchFact { label: qsTr("Status"); value: qsTr("Noch nicht angeschlossen · kein MCP-Zugriff aktiv") }
        WorkbenchHint { text: qsTr("Nach Fertigstellung: bewusste Gesamtfreigabe je Computer, jederzeit widerrufbar. Geräteaktionen verwenden dieselbe Verbindung und Konfliktentscheidung wie die Oberfläche.") }
        WorkbenchRule {}
        WorkbenchLabel { text: qsTr("NFC-Systemreader"); font.pixelSize: 21 }
        WorkbenchFact { label: qsTr("macOS"); value: qsTr("Integration und unabhängiger Kartentest noch offen") }
        WorkbenchFact { label: qsTr("Hilfsprogramme auf dem Flipper"); value: qsTr("Jede Installation und jede neue Version benötigt eine ausdrückliche Freigabe. Keine automatische Installation.") }
    }
    ColumnLayout {
        visible: settings.tab === 4; Layout.fillWidth: true; spacing: 22
        WorkbenchLabel { text: "uFlipper"; font.pixelSize: 30 }
        WorkbenchFact { label: qsTr("Version"); value: App.version + " · " + App.commit }
        WorkbenchFact { label: qsTr("Entwicklung"); value: "Joris Conrad" }
        WorkbenchFact { label: qsTr("Lizenz"); value: qsTr("GNU General Public License v3 · GPLv3") }
        WorkbenchFact { label: qsTr("Credits"); value: qsTr("Auf Basis von qFlipper von Flipper Devices. Momentum-Firmware und Community-Werkzeuge bleiben eigenständige Projekte.") }
        RowLayout {
            WorkbenchButton { text: "qFlipper"; onClicked: Qt.openUrlExternally("https://github.com/flipperdevices/qFlipper") }
            WorkbenchButton { text: "Momentum"; onClicked: Qt.openUrlExternally("https://github.com/Next-Flip/Momentum-Firmware") }
            WorkbenchButton { text: "GPLv3"; onClicked: Qt.openUrlExternally("https://www.gnu.org/licenses/gpl-3.0.html") }
        }
        WorkbenchFact { label: qsTr("Kennung"); value: "com.jorisconrad.flipperzeroutil" }
        WorkbenchHint { text: qsTr("Entwicklungsversion. Noch keine öffentliche Kernabnahme oder vollständige Geräte-/Integrationsprüfung.") }
    }
}
