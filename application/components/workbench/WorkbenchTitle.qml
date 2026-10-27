import QtQuick 2.15
import QtQuick.Layouts 1.15
ColumnLayout {
    property alias title: heading.text
    property alias subtitle: subheading.text
    spacing: 8
    Layout.fillWidth: true
    WorkbenchLabel { id: heading; font.pixelSize: 28; Layout.fillWidth: true }
    WorkbenchLabel { id: subheading; font.pixelSize: 16; opacity: 0.7; wrapMode: Text.Wrap; Layout.fillWidth: true }
}
