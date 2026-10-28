import QtQuick 2.15
import QtQuick.Layouts 1.15
import QFlipper 1.0
import ".."
Item {
    objectName: "filesPage"
    readonly property bool ready: Backend.backendState === ApplicationBackend.Ready
    onReadyChanged: if (ready) Backend.fileManager.refresh()
    Component.onCompleted: if (ready) Backend.fileManager.refresh()
    Component.onDestruction: if (Backend.backendState === ApplicationBackend.Ready) Backend.screenStreamer.isEnabled = true
    WorkbenchGrid { anchors.fill: parent }
    ColumnLayout {
        anchors.fill: parent; anchors.margins: 24; spacing: 16
        WorkbenchTitle { title: qsTr("Dateien"); subtitle: qsTr("Interner Speicher und SD-Karte – bestehende qFlipper-Dateiverwaltung.") }
        WorkbenchHint { visible: !ready; text: qsTr("Dateien sind verfügbar, sobald der Flipper verbunden und bereit ist.") }
        FileManager {
            id: files
            visible: ready
            Layout.fillWidth: true; Layout.fillHeight: true
            messageDialog: messages; confirmationDialog: confirmations
        }
        Item { visible: !ready; Layout.fillHeight: true }
    }
    MessageDialog { id: messages; parent: files.parent }
    ConfirmationDialog { id: confirmations; parent: files.parent }
    ProgressDialog {
        id: progress; parent: files.parent
        title: qsTr("Dateiübertragung"); text: qsTr("Vorgang läuft …")
        value: Backend.deviceState ? Backend.deviceState.progress : -1
        indeterminate: value < 0
    }
    Connections { target: Backend.fileManager; function onIsBusyChanged() { if (Backend.fileManager.isBusy) progress.open(); else progress.close() } }
}
