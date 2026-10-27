import QtQuick 2.15
import QtQuick.Layouts 1.15
import Theme 1.0
import QFlipper 1.0
import ".."
WorkbenchSplit {
    id: screen
    objectName: "screenPage"
    showDetails: false
    Component.onCompleted: if (Backend.backendState === ApplicationBackend.Ready) Backend.screenStreamer.isEnabled = true
    WorkbenchTitle { title: qsTr("Gerätebildschirm"); subtitle: qsTr("Den Flipper über die bestehende USB-Fernsteuerung bedienen.") }
    WorkbenchHint { visible: !Backend.deviceState || Backend.deviceState.isRecoveryMode; text: qsTr("Einen Flipper im normalen Betrieb per USB verbinden.") }
    RowLayout {
        Layout.fillWidth: true; spacing: 32
        Rectangle {
            Layout.fillWidth: true; implicitHeight: canvas.height + 32
            radius: 6; color: "#120900"; border.color: Theme.color.mediumorange2
            ScreenCanvas {
                id: canvas
                anchors.centerIn: parent
                foregroundColor: "black"; backgroundColor: Theme.color.lightorange2
                zoomFactor: Math.max(1, Math.min(5, Math.floor((parent.width - 32) / 128)))
                frame: Backend.screenStreamer.screenFrame
            }
        }
        DirectionalKeypad {
            enabled: Backend.backendState === ApplicationBackend.Ready
            onInputEvent: function(key, type) { Backend.screenStreamer.sendInputEvent(key, type) }
        }
    }
    WorkbenchFact { label: qsTr("Verbindung"); value: Backend.deviceState && Backend.deviceState.isOnline ? qsTr("USB verbunden") : qsTr("Nicht verbunden") }
}
