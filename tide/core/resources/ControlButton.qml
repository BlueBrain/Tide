import QtQuick 2.0
import "style.js" as Style

Item {
    property alias image: image.source

    width: Style.buttonsSize
    height: Style.buttonsSize
    Image {
        id: image
        width: Style.buttonsImageSize
        height: Style.buttonsImageSize
        anchors.centerIn: parent
        // Force redraw the SVG
        sourceSize.width: width
        sourceSize.height: height
    }
}
