import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Item {
    id: sideControl

    width: childrenRect.width

    anchors.verticalCenter: parent.verticalCenter
    anchors.left: parent.left

    // Can't be made a top-level item; this results in unexplained incorrect
    // centering of buttons inside the Canvas.
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
        LaunchButton {
            visible: !displaygroup.hasFocusedWindows
                     && !displaygroup.hasFullscreenWindows
            size: Style.buttonsSizeLarge
        }
        ClockButton {
            visible: displaygroup.hasFocusedWindows
                     && !displaygroup.hasFullscreenWindows
            size: Style.buttonsSizeLarge
        }
        ContentActionButton {
        }
        HorizontalButtonSeparator {
            width: Style.buttonsSizeLarge
        }
        FocusButton {
            id: focusButton
            visible: !displaygroup.empty && !displaygroup.hasFocusedWindows
                     && !displaygroup.hasFullscreenWindows
            size: Style.buttonsSizeLarge
            onClicked: sidecontroller.toggleFocusAll()
        }
        LockButton {
            visible: displaygroup.empty
            size: Style.buttonsSizeLarge
        }
        ExitButton {
            id: exitButton
            visible: displaygroup.hasFocusedWindows
                     || displaygroup.hasFullscreenWindows
            size: Style.buttonsSizeLarge
        }
    }
}
