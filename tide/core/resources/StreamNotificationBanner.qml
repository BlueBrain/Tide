import QtQuick 2.0
import Tide 1.0
import "qrc:/qml/core/style.js" as Style

Rectangle {
    id: banner
    width: row.width + 2 * Style.buttonsPadding + leftPadding
    height: row.height + 2 * Style.buttonsPadding
    radius: height / 2
    color: Style.surfaceControlsColor

    property real leftPadding: 0

    Row {
        id: row
        anchors.left: parent.left
        anchors.leftMargin: banner.leftPadding
        anchors.verticalCenter: parent.verticalCenter
        spacing: Style.buttonsPadding
        Text {
            text: modelData
            anchors.verticalCenter: parent.verticalCenter
            width: Style.buttonsSize * 8
            font.pixelSize: Style.buttonsSize / 2
            minimumPixelSize: Style.buttonsSize / 5
            font.family: "Verdana"
            fontSizeMode: Text.VerticalFit
            wrapMode: Text.Wrap
            maximumLineCount: 2
        }
        ControlButton {
            image: "qrc:/img/close.svg"
            anchors.verticalCenter: parent.verticalCenter
            onClicked: lock.rejectStream(modelData)
        }
        ControlButton {
            image: "qrc:/img/tick.svg"
            anchors.verticalCenter: parent.verticalCenter
            onClicked: lock.acceptStream(modelData)
        }
    }
}
