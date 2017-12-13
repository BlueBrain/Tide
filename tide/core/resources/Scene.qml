import QtQuick 2.0
import Tide 1.0
import "qrc:/qml/core/style.js" as Style

Item {
    id: sceneitem

    property alias sideControl: sideControl
    property alias streamNotificationArea: streamNotificationArea

    SideControl {
        id: sideControl
        z: Style.sideControlZorder
        visible: options.showControlArea
    }

    StreamNotificationArea {
        id: streamNotificationArea
        anchors.top: parent.verticalCenter
        anchors.left: parent.left
    }
}
