import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

SideButton {
    id: sideControl
    color: Style.sideButtonColor
    height: Style.sideButtonRelHeight * displaygroup.height
    property real innerMargin: Style.sideButtonInnerMargin
    delegateHeight: (1.0 - innerMargin) * 2.5 * width

    property Component buttonDelegate: Item {
    }

    delegate: Column {
        anchors.fill: parent
        anchors.margins: sideControl.innerMargin * parent.width
        Image {
            source: displaygroup.hasFullscreenWindows
                    || displaygroup.hasFocusedWindows ? "qrc:/img/exit.svg" : "qrc:/img/launch.svg"
            width: parent.width
            height: width
            anchors.horizontalCenter: parent.horizontalCenter
            Loader {
                property int buttonIndex: displaygroup.hasFullscreenWindows ? 2 : displaygroup.hasFocusedWindows ? 1 : 0
                anchors.fill: parent
                sourceComponent: buttonDelegate
            }
        }
        HorizontalButtonSeparator {
            width: parent.width
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
                delegate: ContentActionButton {
                    width: bottomItem.width
                    height: bottomItem.height
                    imageRelSize: 1.0 // remove padding
                }
                model: displaygroup.fullscreenWindow ? displaygroup.fullscreenWindow.content.actions : undefined
            }

            Image {
                source: lock.locked ? "qrc:/img/lock.svg" : "qrc:/img/unlock.svg"
                width: parent.width
                height: width
                anchors.horizontalCenter: parent.horizontalCenter
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
