import QtQuick 2.15
import Theme 1.0
import "WorkbenchGeometry.js" as Geometry
Text {
    color: Theme.color.lightorange2
    font.family: "Share Tech"
    font.pixelSize: 17
    lineHeightMode: Text.FixedHeight
    lineHeight: Geometry.snapUp(font.pixelSize * 1.1)
    renderType: Text.NativeRendering
    textFormat: Text.PlainText
    elide: Text.ElideRight
}
