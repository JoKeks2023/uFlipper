import QtQuick 2.15
import QtQuick.Controls 2.15
import Theme 1.0
Button {
    id: control
    property bool navigation: false
    property bool primary: false
    implicitHeight: navigation ? 36 : 34
    implicitWidth: Math.max(80, label.implicitWidth + 26)
    leftPadding: 12; rightPadding: 12
    Accessible.name: text
    ToolTip.visible: hovered && label.truncated
    ToolTip.text: text
    ToolTip.delay: Theme.timing.toolTipDelay
    background: Rectangle {
        radius: 5
        color: control.primary && control.enabled ? Theme.color.lightorange1 :
               control.checked ? Theme.color.darkorange1 : control.down || control.hovered ? "#321a0b" :
               control.navigation ? "transparent" : "#120900"
        border.width: control.navigation && !control.activeFocus ? 0 : 1
        border.color: control.activeFocus ? Theme.color.lightorange1 : Theme.color.mediumorange2
        opacity: control.enabled ? 1 : 0.45
    }
    contentItem: WorkbenchLabel {
        id: label
        text: control.text
        color: control.primary && control.enabled ? "#100700" : Theme.color.lightorange2
        opacity: control.enabled ? 1 : 0.5
        horizontalAlignment: control.navigation ? Text.AlignLeft : Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
