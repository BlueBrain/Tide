import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Item {
    id: sideControl

    width: childrenRect.width

    anchors.verticalCenter: parent.verticalCenter
    anchors.left: parent.left

    // Can't be made a top-level item; this results in unexplained incorrect
    // centering of buttons inside the Canevas.
    SideButton {
        id: buttonShape
        width: buttons.width + 2 * Style.buttonsPaddingLarge
        color: Style.surfaceControlsColor
        dropShadow: true
    }
    Column {
        id: buttons
        anchors.centerIn: buttonShape
        anchors.horizontalCenterOffset: -0.5 * buttonShape.dropShadowWidth
        ExitButton {
            id: exitButton
            size: Style.buttonsSizeLarge
        }
        LaunchButton {
            visible: !exitButton.visible
            size: Style.buttonsSizeLarge
        }
        HorizontalButtonSeparator {
            width: Style.buttonsSizeLarge
        }
        ContentActionButton {
            id: contentAction
        }
        LockButton {
            visible: !contentAction.visible
            size: Style.buttonsSizeLarge
        }
    }
}
