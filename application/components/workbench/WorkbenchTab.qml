import QtQuick 2.15
import Theme 1.0
WorkbenchButton {
    id: tab
    navigation: true
    implicitHeight: 40
    background: Item {
        Rectangle {
            anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
            height: tab.checked ? 2 : 1
            visible: tab.checked || tab.hovered || tab.activeFocus
            color: tab.checked ? Theme.color.lightorange1 : Theme.color.mediumorange2
        }
    }
}
