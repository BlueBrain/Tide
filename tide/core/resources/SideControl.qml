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
            source: displaygroup.hasFullscreenWindows ? "qrc:/img/exit.svg" :
                    displaygroup.hasFocusedWindows ? "qrc:/img/focus.svg" :
                                                     "qrc:/img/launch.svg"
            width: parent.width
            height: width
            anchors.horizontalCenter: parent.horizontalCenter
            Loader {
                property int buttonIndex: displaygroup.hasFullscreenWindows ? 2 :
                                          displaygroup.hasFocusedWindows ? 1 : 0
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
        Item {
            id: bottomItem
            width: parent.width
            height: width
            ListView {
                id: contentActionButton
                anchors.fill: parent
                orientation: ListView.Vertical
                interactive: false // disable flickable behaviour
                visible: count > 0
                delegate: WindowControlsDelegate {
                    width: bottomItem.width
                    height: bottomItem.height
                    imageRelSize: 1.0 // remove padding
                    MouseArea {
                        anchors.fill: parent
                        onClicked: action.trigger()
                    }
                }
                model: displaygroup.fullscreenWindow ?
                           displaygroup.fullscreenWindow.content.actions : undefined
            }
            Clock {
                displayedHeight: parent.width
                anchors.horizontalCenter: parent.horizontalCenter
                showSeconds: false
                visible: !contentActionButton.visible
                Loader {
                    property int buttonIndex: 3
                    anchors.fill: parent
                    sourceComponent: buttonDelegate
                }
            }
        }
    }
}
