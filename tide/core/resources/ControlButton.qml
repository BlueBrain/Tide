import QtQuick 2.0
import "style.js" as Style

Item {
    property alias image: image.source
    property real imageRelSize: Style.buttonsImageRelSize

    width: Style.buttonsSize
    height: Style.buttonsSize
    Image {
        id: image
        width: parent.width * imageRelSize
        height: parent.height * imageRelSize
        anchors.centerIn: parent
        // Force redraw the SVG
        sourceSize.width: width
        sourceSize.height: height
    }
}
