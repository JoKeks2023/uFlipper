import QtQuick 2.15
import QtQuick.Layouts 1.15
import Theme 1.0
Rectangle {
    property alias text: label.text
    implicitHeight: label.implicitHeight + 24
    radius: 5; color: "#321b0d"
    Layout.fillWidth: true
    WorkbenchLabel {
        id: label
        x: 12; y: 12; width: parent.width - 24
        font.pixelSize: 16; wrapMode: Text.Wrap
    }
}
