import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Theme 1.0
import "WorkbenchGeometry.js" as Geometry
Item {
    id: split
    default property alias body: mainColumn.data
    property alias details: detailColumn.data
    property bool showDetails: true
    RowLayout {
        anchors.fill: parent; spacing: 0
        ScrollView {
            id: mainScroll
            padding: 0; outlined: false
            Layout.fillWidth: true; Layout.fillHeight: true
            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            Flickable {
                id: mainContent
                objectName: "workbenchScrollContent"
                clip: true; pixelAligned: true
                contentWidth: width
                contentHeight: mainColumn.implicitHeight + 2 * Geometry.padding
                WorkbenchGrid {
                    width: mainContent.width
                    height: Math.max(mainContent.height, mainContent.contentHeight)
                }
                ColumnLayout {
                    id: mainColumn
                    objectName: "workbenchMainColumn"
                    x: Geometry.padding; y: Geometry.padding
                    width: Geometry.snapDown(mainContent.width - 2 * Geometry.padding)
                    spacing: Geometry.gap
                }
            }
        }
        Rectangle {
            Layout.fillHeight: true; Layout.preferredWidth: Geometry.detailWidth
            visible: split.showDetails
            color: "#120900"
            Rectangle { width: 1; height: parent.height; color: Theme.color.mediumorange2 }
            ScrollView {
                id: detailScroll
                padding: 0; outlined: false
                anchors.fill: parent; clip: true
                contentWidth: availableWidth
                contentHeight: detailColumn.implicitHeight + 2 * Geometry.padding
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                ColumnLayout {
                    id: detailColumn
                    x: Geometry.padding; y: Geometry.padding
                    width: Geometry.snapDown(detailScroll.availableWidth - 2 * Geometry.padding)
                    spacing: Geometry.gap
                }
            }
        }
    }
}
