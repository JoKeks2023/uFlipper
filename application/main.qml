import QtQuick 2.15
import QtQuick.Window 2.15
import QFlipper 1.0
import "components"
Window {
    id: root
    visible: true
    flags: Qt.Window | Qt.FramelessWindowHint
    title: Qt.application.displayName
    minimumWidth: 1060
    minimumHeight: 680
    width: Math.max(minimumWidth, Math.min(1320, Screen.desktopAvailableWidth - 80))
    height: Math.max(minimumHeight, Math.min(850, Screen.desktopAvailableHeight - 80))
    color: "#000000"
    MainWindow { id: mainWindow }
    onClosing: function(close) { if (mainWindow.maintenance) close.accepted = false }
    Component.onCompleted: App.messageReceived.connect(root.alert)
    Repeater {
        model: [Qt.LeftEdge, Qt.RightEdge, Qt.TopEdge, Qt.BottomEdge, Qt.LeftEdge | Qt.TopEdge, Qt.RightEdge | Qt.TopEdge, Qt.LeftEdge | Qt.BottomEdge, Qt.RightEdge | Qt.BottomEdge]
        MouseArea {
            required property int modelData
            readonly property bool isLeftEdge: (modelData & Qt.LeftEdge) !== 0
            readonly property bool isRightEdge: (modelData & Qt.RightEdge) !== 0
            readonly property bool isTopEdge: (modelData & Qt.TopEdge) !== 0
            readonly property bool isBottomEdge: (modelData & Qt.BottomEdge) !== 0
            x: isRightEdge ? root.width - width : 0
            y: isBottomEdge ? root.height - height : 0
            width: isLeftEdge || isRightEdge ? 4 : root.width
            height: isTopEdge || isBottomEdge ? 4 : root.height
            z: 100
            enabled: root.visibility !== Window.Maximized
            cursorShape: (isLeftEdge || isRightEdge) && (isTopEdge || isBottomEdge) ? (isLeftEdge && isTopEdge || isRightEdge && isBottomEdge ? Qt.SizeFDiagCursor : Qt.SizeBDiagCursor) : isLeftEdge || isRightEdge ? Qt.SizeHorCursor : Qt.SizeVerCursor
            onPressed: root.startSystemResize(modelData)
        }
    }
}
