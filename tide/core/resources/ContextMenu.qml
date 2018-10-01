import QtQuick 2.0
import "qrc:/qml/core/style.js" as Style

Rectangle {
    color: Style.surfaceControlsColor
    height: row.height + 2 * Style.buttonsPaddingLarge
    width: row.width + 2 * Style.buttonsPaddingLarge

    Image {
        id: indicator
        source: "qrc:/img/context_menu_indicator.svg"
        width: height
        height: parent.height
        anchors.horizontalCenter: parent.left
        anchors.verticalCenter: parent.top
        // Force redraw the SVG
        sourceSize.width: width
        sourceSize.height: height
    }

    Row {
        id: row
        anchors.centerIn: parent
        spacing: Style.buttonsPaddingLarge

        FocusButton {
            size: Style.buttonsSizeLarge
            onClicked: contextmenucontroller.focus()
        }
        VerticalButtonSeparator {
            height: Style.buttonsSizeLarge
            visible: lockButton.visible
        }
        LockButton {
            id: lockButton
            size: Style.buttonsSizeLarge
            visible: !copyButton.visible
        }
        VerticalButtonSeparator {
            height: Style.buttonsSizeLarge
            visible: copyButton.visible
        }
        CopyButton {
            id: copyButton
            size: Style.buttonsSizeLarge
            itemCount: displaygroup.selectedUris.length
            onClicked: contextmenucontroller.copy(displaygroup.selectedUris)
            visible: itemCount > 0
        }
        VerticalButtonSeparator {
            height: Style.buttonsSizeLarge
            visible: pasteButton.visible
        }
        PasteButton {
            id: pasteButton
            size: Style.buttonsSizeLarge
            itemCount: contextmenu.copiedUris.length
            onClicked: contextmenucontroller.paste()
            visible: itemCount > 0
        }
    }
}
