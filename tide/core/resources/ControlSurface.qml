import QtQuick 2.0
import Tide 1.0
import "qrc:/qml/core/style.js" as Style

Item {
    id: controlSurface

    signal openLauncher

    SideControl {
        id: sideControl
        z: Style.sideControlZorder
        visible: options.showControlArea
        onOpenLauncher: controlSurface.openLauncher()
    }

    StreamNotificationArea {
        z: Style.streamNotificationZorder
        anchors.top: parent.verticalCenter
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        leftPadding: sideControl.width
    }

    ContextMenu {
        z: Style.sideControlZorder
        x: contextmenu.position.x
        y: contextmenu.position.y
        visible: contextmenu.visible
    }
}
