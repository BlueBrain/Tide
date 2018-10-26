import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Item {
    height: Style.buttonsSize
    width: 0.15 * height
    Image {
        source: "qrc:/img/dots.svg"
        width: parent.height
        anchors.centerIn: parent
        rotation: 90
    }
}
