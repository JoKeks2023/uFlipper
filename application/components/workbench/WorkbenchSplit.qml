import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Theme 1.0
Item {
    id: split
    default property alias body: mainColumn.data
    property alias details: detailColumn.data
    property bool showDetails: true
    RowLayout {
        anchors.fill: parent; spacing: 0
        ScrollView {
                padding: 0
                outlined: false
            id: mainScroll
            Layout.fillWidth: true; Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth
            contentHeight: mainColumn.implicitHeight + 48
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ColumnLayout {
                id: mainColumn
                x: 24; y: 24; width: mainScroll.availableWidth - 48
                spacing: 18
            }
        }
        Rectangle {
            Layout.fillHeight: true; Layout.preferredWidth: 242
            visible: split.showDetails
            color: "#120900"
            Rectangle { width: 1; height: parent.height; color: Theme.color.mediumorange2 }
            ScrollView {
                padding: 0
                outlined: false
                id: detailScroll
                anchors.fill: parent; clip: true
                contentWidth: availableWidth; contentHeight: detailColumn.implicitHeight + 48
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                ColumnLayout {
                    id: detailColumn
                    x: 20; y: 26; width: detailScroll.availableWidth - 40; spacing: 18
                }
            }
        }
    }
}
