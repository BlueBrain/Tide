import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Canvas {
    id: sideButton
    width: Style.windowSideButtonWidth
    height: Style.windowSideButtonNarrowHeight + 2 * Style.windowSideButtonTriangleHeight

    property color color: "lightgray"
    onColorChanged: requestPaint()

    property bool flipRight: false
    anchors.verticalCenter: parent.verticalCenter
    anchors.right: flipRight ? parent.right : undefined
    anchors.left: flipRight ? undefined : parent.left
    rotation: flipRight ? 180 : 0

    default property alias delegate : iconDelegate.sourceComponent
    property real delegateOverflow: 0.0 // The icon can overlap the window border

    onPaint: {
        var ctx = getContext("2d");
        ctx.save();
        ctx.clearRect(0, 0, width, height)

        var verticalCenter = 0.5 * height
        var delta = 0.5 * Style.windowSideButtonNarrowHeight

        ctx.fillStyle = color;
        ctx.beginPath();
        ctx.moveTo(0, 0);
        ctx.lineTo(0, height);
        ctx.lineTo(width, verticalCenter + delta);
        ctx.lineTo(width, verticalCenter - delta);
        ctx.closePath();
        ctx.fill();

        ctx.restore();
    }

    Loader {
       id: iconDelegate
       anchors.verticalCenter: parent.verticalCenter
       anchors.left: parent.left
       anchors.leftMargin: -delegateOverflow
       width: parent.width + delegateOverflow
       height: width
    }
}
