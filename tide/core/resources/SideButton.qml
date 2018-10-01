import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Canvas {
    width: Style.windowSideButtonWidth
    height: width / Style.sideButtonAspectRatio

    property color color: Style.sideButtonColor
    onColorChanged: requestPaint()

    property bool dropShadow: false
    property real dropShadowWidth: dropShadow ? width / 20 : 0

    property bool flipRight: false
    anchors.verticalCenter: parent.verticalCenter
    anchors.right: flipRight ? parent.right : undefined
    anchors.left: flipRight ? undefined : parent.left
    rotation: flipRight ? 180 : 0

    default property alias delegate: iconDelegate.sourceComponent
    property real delegateOverflow: 0.0 // The icon can overlap the window border
    property real delegateHeight: 0.0 // The icon can be non-square

    property alias icon: icon.source

    onPaint: {
        var ctx = getContext("2d")
        ctx.save()
        ctx.clearRect(0, 0, width, height)

        var verticalCenter = 0.5 * height
        var narrowHeight = height * Style.sideButtonRelNarrowHeight
        var delta = 0.5 * narrowHeight

        if (dropShadow) {
            var dropShadowHeight = 2 * dropShadowWidth

            ctx.fillStyle = "black"
            ctx.beginPath()
            ctx.moveTo(0, 0)
            ctx.lineTo(0, height)
            ctx.lineTo(width, verticalCenter + delta + dropShadowHeight)
            ctx.lineTo(width, verticalCenter - delta + dropShadowHeight)
            ctx.closePath()
            ctx.fill()
        }

        ctx.fillStyle = color
        ctx.beginPath()
        ctx.moveTo(0, 0)
        ctx.lineTo(0, height)
        ctx.lineTo(width - dropShadowWidth, verticalCenter + delta)
        ctx.lineTo(width - dropShadowWidth, verticalCenter - delta)
        ctx.closePath()
        ctx.fill()

        ctx.restore()
    }

    Image {
        id: icon
        width: parent.width + delegateOverflow
        height: width
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: -delegateOverflow
        // Force redraw the SVG
        sourceSize.width: width
        sourceSize.height: height
    }

    Loader {
        id: iconDelegate
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: -delegateOverflow
        width: parent.width + delegateOverflow
        height: delegateHeight ? delegateHeight : width
    }
}
