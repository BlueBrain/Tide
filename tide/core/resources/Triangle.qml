import QtQuick 2.0

Canvas {
    id: triangle
    antialiasing: true

    property int triangleWidth: 0.5 * width
    property int triangleHeight: 0.5 * height
    property int lineWidth: 3
    property color strokeColor: "black"
    property color fillColor: "black"
    property bool stroke: true
    property bool fill: true

    onTriangleWidthChanged: requestPaint()
    onTriangleHeightChanged: requestPaint()
    onLineWidthChanged: requestPaint()
    onStrokeColorChanged: requestPaint()
    onFillColorChanged: requestPaint()
    onStrokeChanged: requestPaint()
    onFillChanged: requestPaint()

    onPaint: {
        var ctx = getContext("2d")
        ctx.save()
        ctx.clearRect(0, 0, triangle.width, triangle.height)
        ctx.lineWidth = triangle.lineWidth
        ctx.strokeStyle = triangle.strokeColor
        ctx.fillStyle = triangle.fillColor
        ctx.lineJoin = "round"

        // translate the triangle's origin (0,0) so that it looks centered
        ctx.translate((0.45 * (triangle.width - triangleWidth)),
                      (0.5 * (triangle.height - triangleHeight)))

        ctx.beginPath()
        ctx.moveTo(0, triangleHeight / 2)
        ctx.lineTo(triangleWidth, 0)
        ctx.lineTo(triangleWidth, triangleHeight)
        ctx.closePath()

        if(triangle.fill)
            ctx.fill()
        if(triangle.stroke)
            ctx.stroke()
        ctx.restore()
    }
}
