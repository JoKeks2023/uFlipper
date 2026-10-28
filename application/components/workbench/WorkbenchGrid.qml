import QtQuick 2.15
import "WorkbenchGeometry.js" as Geometry

Canvas {
    id: grid
    objectName: "workbenchGrid"
    property real originX: Geometry.padding
    property real originY: Geometry.padding
    property color minorColor: "#0b0602"
    property color majorColor: "#241508"
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onOriginXChanged: requestPaint()
    onOriginYChanged: requestPaint()
    onMinorColorChanged: requestPaint()
    onMajorColorChanged: requestPaint()
    onPaint: {
        const ctx = getContext("2d")
        ctx.reset()
        ctx.lineWidth = 1
        // Share the layout's logical-pixel origin; scroll via the same content item.
        for (let pass = 0; pass < 2; ++pass) {
            const step = Geometry.unit * (pass === 0 ? 1 : 5)
            ctx.strokeStyle = pass === 0 ? minorColor : majorColor
            ctx.beginPath()
            for (let x = originX % step; x < width; x += step) {
                ctx.moveTo(Math.round(x) + 0.5, 0)
                ctx.lineTo(Math.round(x) + 0.5, height)
            }
            for (let y = originY % step; y < height; y += step) {
                ctx.moveTo(0, Math.round(y) + 0.5)
                ctx.lineTo(width, Math.round(y) + 0.5)
            }
            ctx.stroke()
        }
    }
}
