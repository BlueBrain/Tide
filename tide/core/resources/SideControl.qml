import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Item {
    id: sideControl

    signal openLauncher

    width: childrenRect.width

    anchors.verticalCenter: parent.verticalCenter
    anchors.left: parent.left

    // Can't be made a top-level item; this results in unexplained incorrect
    // centering of buttons inside the Canevas.
    SideButton {
        id: buttonShape
        width: buttons.width + 2 * Style.buttonsPaddingLarge
        color: Style.surfaceControlsColor
    }
    Column {
        id: buttons
        anchors.centerIn: buttonShape
        ExitControlButton {
            id: exitButton
            size: Style.buttonsSizeLarge
        }
        LaunchControlButton {
            visible: !exitButton.visible
            onOpenLauncher: sideControl.openLauncher()
            size: Style.buttonsSizeLarge
        }
        HorizontalButtonSeparator {
            width: Style.buttonsSizeLarge
        }
        ContentActionsButtons {
            id: contentAction
            size: Style.buttonsSizeLarge
            visible: count > 0
            model: displaygroup.fullscreenWindow ? displaygroup.fullscreenWindow.content.actions : undefined
        }
        LockControlButton {
            visible: !contentAction.visible
            size: Style.buttonsSizeLarge
        }
    }
}
