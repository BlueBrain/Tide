import QtQuick 2.0
import Tide 1.0
import "qrc:/qml/core/style.js" as Style

Rectangle {
    function fuzzyCompare(value1, value2) {
        return Math.abs(value1 - value2) <= 0.0000001
    }

    function hasZoom(rect) {
        return !fuzzyCompare(rect.width, 1.0) || !fuzzyCompare(rect.height, 1.0)
    }

    property bool vertical: contentwindow.content.aspectRatio < 1.0

    border.width: Style.zoomContextBorderWidth
    border.color: Style.zoomContextBorderColor
    width: vertical ? Math.min(parent.width * Style.zoomContextSizeRatio,
                               parent.height * Style.zoomContextMaxSizeRatio * contentwindow.content.aspectRatio) :
                      Math.min(parent.height * Style.zoomContextSizeRatio * contentwindow.content.aspectRatio,
                               parent.width * Style.zoomContextMaxSizeRatio)
    color: Style.zoomContextBackgroundColor
    height: width / contentwindow.content.aspectRatio
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.bottomMargin: vertical ? width * Style.zoomContextRelMargin :
                                     height * Style.zoomContextRelMargin
    anchors.leftMargin: anchors.bottomMargin
    visible: options.showZoomContext && hasZoom(contentwindow.content.zoomRect)

    Item {
        id: zoomContextParent
        objectName: "ZoomContextParent"
        anchors.fill: parent
        anchors.margins: parent.border.width
    }

    Rectangle {
        border.width: Style.zoomContextSelectionWidth
        border.color: Style.zoomContextSelectionColor
        color: "transparent"
        x: zoomContextParent.x + contentwindow.content.zoomRect.x * zoomContextParent.width
        y: zoomContextParent.y + contentwindow.content.zoomRect.y * zoomContextParent.height
        width: zoomContextParent.width * contentwindow.content.zoomRect.width
        height: zoomContextParent.height * contentwindow.content.zoomRect.height
    }
}
