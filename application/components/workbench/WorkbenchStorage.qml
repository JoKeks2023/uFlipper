import QtQuick 2.15
import QtQuick.Layouts 1.15
import Theme 1.0
ColumnLayout {
    id: storage
    property string title
    property var info: ({})
    function bytes(value) {
        if (value >= 1073741824) return (value / 1073741824).toLocaleString(Qt.locale(), 'f', 1) + " GiB"
        if (value >= 1048576) return (value / 1048576).toLocaleString(Qt.locale(), 'f', 1) + " MiB"
        return (value / 1024).toLocaleString(Qt.locale(), 'f', 0) + " KiB"
    }
    spacing: 10; Layout.fillWidth: true
    RowLayout {
        Layout.fillWidth: true
        WorkbenchLabel { text: storage.title; Layout.fillWidth: true }
        WorkbenchLabel {
            font.pixelSize: 14
            text: storage.info.available ? storage.bytes(storage.info.usedBytes) + " / " + storage.bytes(storage.info.totalBytes) : qsTr("Nicht verfügbar")
            opacity: 0.7
        }
    }
    Rectangle {
        Layout.fillWidth: true; implicitHeight: 5; radius: 2; color: "#3a1f10"
        Rectangle {
            height: parent.height; radius: 2; color: Theme.color.lightorange1
            width: storage.info.available && storage.info.totalBytes > 0 ? parent.width * Math.max(0, Math.min(1, storage.info.usedBytes / storage.info.totalBytes)) : 0
        }
    }
    WorkbenchLabel {
        Layout.fillWidth: true; font.pixelSize: 13; opacity: 0.65; wrapMode: Text.Wrap
        visible: !storage.info.available || !!storage.info.stale
        text: storage.info.error || (storage.info.available ? qsTr("Wert ist veraltet") : qsTr("Wird bei Geräteverbindung gelesen"))
    }
}
