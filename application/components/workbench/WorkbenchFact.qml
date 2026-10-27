import QtQuick 2.15
import QtQuick.Layouts 1.15
ColumnLayout {
    property alias label: caption.text
    property alias value: valueLabel.text
    spacing: 5
    Layout.fillWidth: true
    WorkbenchLabel { id: caption; font.pixelSize: 14; opacity: 0.65; Layout.fillWidth: true }
    WorkbenchLabel { id: valueLabel; wrapMode: Text.Wrap; Layout.fillWidth: true }
}
