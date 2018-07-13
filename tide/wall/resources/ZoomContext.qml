import QtQuick 2.0
import Tide 1.0
import "qrc:/qml/core/style.js" as Style

Rectangle {
    property bool vertical: window.content.aspectRatio < 1.0

    border.width: Style.zoomContextBorderWidth
    border.color: Style.zoomContextBorderColor
    width: vertical ? Math.min(parent.width * Style.zoomContextSizeRatio,
                               parent.height * Style.zoomContextMaxSizeRatio * window.content.aspectRatio) :
                      Math.min(parent.height * Style.zoomContextSizeRatio * window.content.aspectRatio,
                               parent.width * Style.zoomContextMaxSizeRatio)
    color: Style.zoomContextBackgroundColor
    height: width / window.content.aspectRatio
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.bottomMargin: vertical ? width * Style.zoomContextRelMargin :
                                     height * Style.zoomContextRelMargin
    anchors.leftMargin: anchors.bottomMargin
    visible: options.showZoomContext && window.content.zoomed

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
        x: zoomContextParent.x + window.content.zoomRect.x * zoomContextParent.width
        y: zoomContextParent.y + window.content.zoomRect.y * zoomContextParent.height
        width: zoomContextParent.width * window.content.zoomRect.width
        height: zoomContextParent.height * window.content.zoomRect.height
    }
}
