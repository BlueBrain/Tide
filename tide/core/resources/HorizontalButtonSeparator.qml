import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Item {
    width: Style.buttonsSize
    height: 0.5 * width
    Text {
        text: "........."
        font.pixelSize: parent.height
        anchors.top: parent.top
        anchors.topMargin: -0.4 * font.pixelSize
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
