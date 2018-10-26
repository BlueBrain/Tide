import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Item {
    width: Style.buttonsSize
    height: 0.5 * width
    Image {
        source: "qrc:/img/dots.svg"
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter
    }
}
