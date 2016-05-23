import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

SideButton {
    id: sideControl
    color: Style.sideButtonColor
    height: Style.sideButtonRelHeight * displaygroup.height
    property real innerMargin: 0.15
    delegateHeight: (1.0 - innerMargin) * 2.5 * width

    property Component buttonDelegate: Item{
    }

    delegate: Column {
        anchors.fill: parent
        anchors.margins: sideControl.innerMargin * parent.width
        Image {
            source: "qrc:/img/launch.svg"
            width: parent.width
            height: width
            anchors.horizontalCenter: parent.horizontalCenter
            Loader {
                property int buttonIndex: 0
                anchors.fill: parent
                sourceComponent: buttonDelegate
            }
        }
        Item {
            width: parent.width
            height: 0.5 * parent.width
            Text {
                font.pixelSize: parent.height
                anchors.top: parent.top
                anchors.topMargin: -0.4 * font.pixelSize
                text: "........."
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
        Image {
            source: "qrc:/img/gear.svg"
            width: parent.width
            height: width
            anchors.horizontalCenter: parent.horizontalCenter
            Loader {
                property int buttonIndex: 1
                anchors.fill: parent
                sourceComponent: buttonDelegate
            }
        }
    }
}
