import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Theme 1.0
import QFlipper 1.0
WorkbenchSplit {
    id: tool
    objectName: "toolPage"
    signal navigate(string page)
    property string toolKey: "infrared"
    property int currentTab: 0
    property bool advanced: false
    onToolKeyChanged: currentTab = 0
    readonly property bool infrared: toolKey === "infrared"
    readonly property var definitions: ({
        infrared: [qsTr("Infrarot"), qsTr("Empfangen, Pulszeiten untersuchen und Fernbedienungen verwalten.")],
        subghz: [qsTr("Sub-GHz"), qsTr("Funkprotokolle, Pulse und RSSI – im Rahmen der Flipper-Hardware.")],
        nfc: [qsTr("NFC"), qsTr("Karten lesen und Ergebnisse am Computer auswerten.")],
        rfid: [qsTr("125-kHz-RFID"), qsTr("Gespeicherte Tags und unterstützte LF-Protokolle.")],
        ibutton: [qsTr("iButton / 1-Wire"), qsTr("Schlüssel und 1-Wire-Geräte untersuchen.")],
        gpio: [qsTr("GPIO und Busse"), qsTr("GPIO, UART, I²C und SPI als Werkzeuge am Arbeitsplatz.")],
        apps: [qsTr("Apps und Skripte"), qsTr("Werkzeuge auf dem Flipper und ihre Dateien verwalten.")],
        usb: [qsTr("USB und Bluetooth"), qsTr("Unterstützte Gerätefunktionen und Verbindungsmodi.")]
    })
    readonly property var recordings: Backend.workbench.recordings.filter(function(item) {
        return tool.infrared ? item.kind.indexOf("ir.") === 0 : item.kind.indexOf(tool.toolKey) === 0
    })
    WorkbenchTitle { title: (tool.definitions[tool.toolKey] || tool.definitions.infrared)[0]; subtitle: (tool.definitions[tool.toolKey] || tool.definitions.infrared)[1] }
    RowLayout {
        Layout.fillWidth: true; spacing: 16
        Repeater {
            model: [qsTr("Empfang"), qsTr("Bedienung"), qsTr("Bibliothek")]
            WorkbenchTab {
                objectName: "tool-tab-" + index
                required property string modelData
                required property int index
                text: modelData; checked: tool.currentTab === index
                onClicked: tool.currentTab = index
            }
        }
        Item { Layout.fillWidth: true }
        WorkbenchButton { objectName: "tool-easy-mode"; text: qsTr("Easy Mode"); checkable: true; checked: Preferences.easyMode; onClicked: Preferences.easyMode = checked }
    }
    WorkbenchHint {
        visible: Preferences.easyMode
        text: tool.infrared ? qsTr("Easy Mode · Empfang starten, Fernbedienung auf den Flipper richten und eine Taste drücken. Der direkte Live-Empfang wird noch angeschlossen.") : qsTr("Easy Mode · Die bisherigen Gerätefunktionen kannst du über den Gerätebildschirm nutzen. Direkte Desktop-Werkzeuge folgen schrittweise.")
    }
    WorkbenchLabel { text: tool.currentTab === 0 ? (tool.infrared ? qsTr("Pulsfolge") : qsTr("Live-Daten")) : tool.currentTab === 1 ? qsTr("Am Gerät bedienen") : qsTr("Gespeicherte Dateien"); font.pixelSize: 21 }
    Rectangle {
        Layout.fillWidth: true; implicitHeight: 210
        radius: 5; color: "#100800"; border.color: Theme.color.mediumorange2
        Canvas {
            anchors.fill: parent; anchors.margins: 16
            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()
            onPaint: {
                const ctx = getContext("2d")
                ctx.reset(); ctx.strokeStyle = "#3a1f10"; ctx.lineWidth = 1; ctx.beginPath()
                for (let x = 0; x < width; x += 48) { ctx.moveTo(x, 0); ctx.lineTo(x, height) }
                for (let y = 0; y < height; y += 40) { ctx.moveTo(0, y); ctx.lineTo(width, y) }
                ctx.stroke()
            }
        }
        WorkbenchLabel {
            anchors.centerIn: parent; width: parent.width - 44; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.Wrap
            text: tool.currentTab === 0 ? qsTr("Noch keine Live-Daten\nDirekter Empfang ist in dieser Version noch nicht verfügbar.") : tool.currentTab === 1 ? qsTr("Gerätebildschirm öffnen und das Werkzeug am Flipper auswählen.") : qsTr("Dateien von internem Speicher und SD-Karte im Dateimanager öffnen.")
        }
    }
    RowLayout {
        visible: tool.currentTab !== 0
        WorkbenchButton { text: tool.currentTab === 1 ? qsTr("Gerätebildschirm öffnen") : qsTr("Dateimanager öffnen"); enabled: Backend.backendState === ApplicationBackend.Ready; onClicked: tool.navigate(tool.currentTab === 1 ? "screen" : "library") }
    }
    WorkbenchLabel { text: qsTr("Lokale Aufnahmen"); font.pixelSize: 21 }
    WorkbenchLabel { visible: tool.recordings.length === 0; text: qsTr("Noch keine Aufnahmen für dieses Werkzeug"); opacity: 0.65; wrapMode: Text.Wrap; Layout.fillWidth: true }
    Repeater {
        model: tool.recordings
        delegate: ColumnLayout {
            required property var modelData
            Layout.fillWidth: true
            RowLayout {
                Layout.fillWidth: true
                WorkbenchFact { label: modelData.kind; value: Qt.formatDateTime(new Date(modelData.started_ms), "dd.MM.yyyy hh:mm") + " · " + modelData.bytes + " Bytes" }
                WorkbenchButton { text: qsTr("Originaldaten"); onClicked: Qt.openUrlExternally(Backend.workbench.recordingUrl(modelData.id)) }
            }
            WorkbenchLabel { Layout.fillWidth: true; text: modelData.reason; font.pixelSize: 14; opacity: 0.65; wrapMode: Text.Wrap }
            WorkbenchRule {}
        }
    }
    details: [
        WorkbenchLabel { text: qsTr("Empfang"); font.pixelSize: 21 },
        WorkbenchLabel { text: qsTr("Modus"); font.pixelSize: 14; opacity: 0.65 },
        WorkbenchComboBox { Layout.fillWidth: true; model: tool.infrared ? [qsTr("RAW-Pulszeiten"), qsTr("Protokoll-Dekodierung")] : [qsTr("Geräteabhängig")]; enabled: tool.infrared; font.family: "Share Tech"; font.pixelSize: 16 },
        WorkbenchFact { label: qsTr("Empfänger"); value: qsTr("Flipper intern") },
        WorkbenchRule {},
        WorkbenchFact { label: qsTr("Sitzung"); value: qsTr("Nicht gestartet") },
        WorkbenchFact { label: qsTr("Aufzeichnung"); value: qsTr("Automatisch bei jedem Live-Start") },
        WorkbenchRule {},
        WorkbenchButton { text: tool.advanced ? qsTr("Parameter ausblenden") : qsTr("Erweiterte Parameter +"); Layout.fillWidth: true; onClicked: tool.advanced = !tool.advanced },
        WorkbenchLabel { visible: tool.advanced; Layout.fillWidth: true; wrapMode: Text.Wrap; font.pixelSize: 15; text: qsTr("Parameter werden mit dem direkten Gerätezugriff verfügbar.") },
        WorkbenchButton { text: qsTr("Empfang starten"); enabled: false; Layout.fillWidth: true },
        WorkbenchLabel { Layout.fillWidth: true; wrapMode: Text.Wrap; font.pixelSize: 14; opacity: 0.65; text: tool.infrared ? qsTr("Pulszeiten, kein optisches Frequenzspektrum. Keine Beispieldaten in der Messansicht.") : qsTr("Direkter Werkzeugzugriff wird noch angeschlossen. Gerätefunktionen bleiben über den Bildschirm erreichbar.") }
    ]
}
