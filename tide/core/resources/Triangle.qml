import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Canvas {
    property color color: Style.sideButtonColor
    onColorChanged: requestPaint()

    onPaint: {
        var ctx = getContext("2d");
        ctx.save();
        ctx.clearRect(0, 0, width, height)

        ctx.fillStyle = color;
        ctx.beginPath();
        ctx.moveTo(0, 0);
        ctx.lineTo(width, 0);
        ctx.lineTo(width, height);
        ctx.closePath();
        ctx.fill();

        ctx.restore();
    }
}
