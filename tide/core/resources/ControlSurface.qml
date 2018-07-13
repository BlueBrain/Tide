import QtQuick 2.0
import Tide 1.0
import "qrc:/qml/core/style.js" as Style

Item {
    id: controlSurface

    signal openLauncher

    Loader {
        active: (typeof groupcontroller !== "undefined") // only load on master
        anchors.fill: parent
        sourceComponent: MultitouchArea {
            anchors.fill: parent
            referenceItem: controlSurface

            onTapAndHold: contextmenucontroller.show(pos)
            onTap: {
                groupcontroller.deselectAll()
                contextmenucontroller.hide()
            }
        }
    }

    SideControl {
        id: sideControl
        z: Style.sideControlZorder
        visible: options.showControlArea
    }

    StreamNotificationArea {
        z: Style.streamNotificationZorder
        anchors.top: parent.verticalCenter
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        leftPadding: sideControl.width
    }

    ContextMenu {
        z: Style.contextMenuZorder
        x: contextmenu.position.x
        y: contextmenu.position.y
        visible: contextmenu.visible
    }
}
