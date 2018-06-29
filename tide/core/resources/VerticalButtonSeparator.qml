import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Item {
    height: Style.buttonsSize
    width: 0.15 * height
    Text {
        text: "........."
        font.pixelSize: 0.4 * parent.height
        anchors.left: parent.left
        anchors.leftMargin: -0.5 * font.pixelSize
        anchors.verticalCenter: parent.verticalCenter
        rotation: 90
    }
}
