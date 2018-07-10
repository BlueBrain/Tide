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

        FocusControlButton {
            size: Style.buttonsSizeLarge
            onClicked: {
                groupcontroller.toggleFocusAll()
                contextmenucontroller.hide()
            }
        }
        VerticalButtonSeparator {
            height: Style.buttonsSizeLarge
            visible: copyButton.visible
        }
        CopyControlButton {
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
        PasteControlButton {
            id: pasteButton
            size: Style.buttonsSizeLarge
            itemCount: contextmenu.copiedUris.length
            onClicked: contextmenucontroller.paste()
            visible: itemCount > 0
        }
    }
}
