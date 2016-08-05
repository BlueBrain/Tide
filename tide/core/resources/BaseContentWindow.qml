import QtQuick 2.0
import QtGraphicalEffects 1.0
import Tide 1.0
import "style.js" as Style

Rectangle {
    id: windowRect

    property bool isBackground: false
    property bool animating: widthAnimation.running || heightAnimation.running
                             || unfocusTransition.running

    property alias backgroundComponent: backgroundLoader.sourceComponent
    property alias contentComponent: contentBackgroundLoader.sourceComponent
    property alias contentArea: contentArea
    property alias focusEffectEnabled: focusEffect.visible
    property alias nextButton: nextButton
    property alias previousButton: previousButton
    property alias resizeCirclesDelegate: resizeCircles.delegate
    property alias titleBar: titleBar
    property alias virtualKeyboard: virtualKeyboard
    property alias windowControlsList: windowControls.listview

    border.color: Style.windowBorderDefaultColor
    border.width: options.showWindowBorders && !isBackground ? Style.windowBorderWidth : 0

    // The window header overlaps with the top window border (when visible)
    property real yOffset: Math.max(border.width, windowHeader.height)

    x: contentArea.posX - border.width
    y: contentArea.posY - yOffset
    width: contentArea.width + border.width * 2
    height: contentArea.height + border.width + yOffset // top + bottom padding
    z: isBackground ? Style.backgroundZOrder : 0

    Loader {
        id: backgroundLoader
        anchors.fill: parent
    }

    Rectangle {
        id: windowHeader
        width: parent.width
        height: visible ? windowHeaderItems.height : 0
        color: windowRect.border.color

        Column {
            id: windowHeaderItems
            width: parent.width
            spacing: -Style.windowTitleControlsOverlap
            Item {
                id: titleBar
                visible: options.showWindowTitles && !windowRect.isBackground
                         && !contentwindow.isPanel
                width: parent.width
                height: Style.windowTitleHeight
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: Style.windowTitleFontSize / 2

                    FontLoader { id: gothamBook; source: "qrc:/fonts/Gotham-Book.otf"; name: "qrc::gotham-book" }

                    elide: Text.ElideRight
                    text: contentwindow.label
                    font { family: "qrc::gotham-book"; pixelSize: Style.windowTitleFontSize }
                }
            }
            Loader {
                id: controlBar
                width: parent.width
                height: Style.windowTitleHeight
                visible: status == Loader.Ready
                source: windowRect.isBackground ? "" : contentwindow.content.qmlControls
            }
        }
    }

    RectangularGlow {
        id: focusEffect
        anchors.fill: contentArea
        cornerRadius: contentArea.radius + glowRadius
        color: Style.windowFocusGlowColor
        glowRadius: Style.windowFocusGlowRadius
        spread: Style.windowFocusGlowSpread
        visible: contentwindow.content.captureInteraction &&
                 contentwindow.state === ContentWindow.NONE
    }

    Rectangle {
        id: contentArea
        property real posX: contentwindow.x
        property real posY: contentwindow.y
        width: contentwindow.width
        height: contentwindow.height

        anchors.bottom: parent.bottom
        anchors.margins: windowRect.border.width
        anchors.horizontalCenter: parent.horizontalCenter
        clip: true
        color: "transparent"

        Loader {
            id: contentBackgroundLoader
            // Note: this loader can't have a size, otherwise it breaks the
            // width/height bindings of loaded content in WallContentWindow.
        }

        Loader {
            id: virtualKeyboard
            source: "qrc:/virtualkeyboard/InputPanel.qml"
            active: contentwindow.content.keyboard.visible
            width: Math.min(Style.keyboardMaxSizePx, Style.keyboardRelSize * parent.width)
            height: 0.25 * width
            anchors.horizontalCenter: parent.horizontalCenter
            y: contentwindow.content.keyboard.visible ? Math.min(0.5 * parent.height, parent.height - height) : parent.height
            opacity:  contentwindow.content.keyboard.visible ? 1.0 : 0.0
            visible: opacity > 0.0
            Behavior on y { NumberAnimation { easing.type: Easing.InOutQuad; duration: 150 }}
            Behavior on opacity { NumberAnimation { easing.type: Easing.InOutQuad; duration: 150 }}
            onStatusChanged: {
                if (status == Loader.Error)
                    source = "qrc:/qml/core/MissingVirtualKeyboard.qml"
                else if( status == Loader.Ready )
                    active = true // Keep the keyboard loaded when hidding it
            }
        }
        Behavior on posX {
            enabled: contentwindow.focused
            NumberAnimation {
                duration: Style.focusTransitionTime
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on posY {
            enabled: contentwindow.focused
            NumberAnimation {
                duration: Style.focusTransitionTime
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on width {
            enabled: contentwindow.focused
            NumberAnimation {
                id: widthAnimation
                duration: Style.focusTransitionTime
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on height {
            enabled: contentwindow.focused
            NumberAnimation {
                id: heightAnimation
                duration: Style.focusTransitionTime
                easing.type: Easing.InOutQuad
            }
        }
    }

    SideButton {
        id: previousButton
        anchors.verticalCenter: contentArea.verticalCenter
        anchors.left: contentArea.left
        color: windowRect.border.color
        delegate: Triangle {
        }
        delegateOverflow: windowRect.border.width
        visible: (contentwindow.controlsVisible || contentwindow.focused ||
                  contentwindow.fullscreen) &&
                 contentwindow.content.page !== undefined &&
                 contentwindow.content.page > 0
    }

    SideButton {
        id: nextButton
        color: windowRect.border.color
        anchors.verticalCenter: contentArea.verticalCenter
        anchors.right: contentArea.right
        delegate: Triangle {
        }
        delegateOverflow: windowRect.border.width
        flipRight: true
        visible: (contentwindow.controlsVisible || contentwindow.focused ||
                  contentwindow.fullscreen) &&
                 contentwindow.content.page !== undefined &&
                 contentwindow.content.page < contentwindow.content.pageCount - 1
    }

    WindowControls {
        id: windowControls
        contentActionsVisible: !controlBar.visible
    }

    ResizeCircles {
        id: resizeCircles
    }

    states: [
        State {
            name: "fullscreen"
            when: contentwindow.fullscreen
            PropertyChanges {
                target: contentArea
                posX: contentwindow.fullscreenCoordinates.x
                posY: contentwindow.fullscreenCoordinates.y
                width: contentwindow.fullscreenCoordinates.width
                height: contentwindow.fullscreenCoordinates.height
            }
            PropertyChanges {
                target: windowRect
                border.width: 0
                z: Style.fullscreenZorder
            }
            PropertyChanges {
                target: windowHeader
                visible: false
            }
        },
        State {
            name: "focused"
            when: contentwindow.focused
            PropertyChanges {
                target: contentArea
                posX: contentwindow.focusedCoordinates.x
                posY: contentwindow.focusedCoordinates.y
                width: contentwindow.focusedCoordinates.width
                height: contentwindow.focusedCoordinates.height
            }
            PropertyChanges {
                target: windowRect
                border.color: Style.windowBorderFocusedColor
                z: Style.focusZorder
            }
        },
        State {
            name: "moving"
            when: contentwindow.state === ContentWindow.MOVING
            PropertyChanges {
                target: windowRect
                border.color: Style.windowBorderMovingColor
            }
        },
        State {
            name: "resizing"
            when: contentwindow.state === ContentWindow.RESIZING
            PropertyChanges {
                target: windowRect
                border.color: Style.windowBorderResizingColor
            }
        },
        State {
            name: "hidden"
            when: contentwindow.state === ContentWindow.HIDDEN
            PropertyChanges {
                target: contentArea
                posX: -contentwindow.width
                posY: 0.5 * displaygroup.height
                width: 0
                height: 0
            }
            PropertyChanges {
                target: windowRect
                opacity: 0
            }
        }
    ]

    transitions: [
        Transition {
            from: "focused"
            to: ""
            id: unfocusTransition
            SequentialAnimation {
                // Slide behind other focused windows for the transition
                PropertyAction {
                    target: windowRect
                    property: "z"
                    value: Style.unfocusZorder
                }
                ParallelAnimation {
                    ColorAnimation { duration: Style.focusTransitionTime }
                    NumberAnimation {
                        target: contentArea
                        properties: "posX,posY,width,height"
                        duration: Style.focusTransitionTime
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        },
        Transition {
            to: "hidden"
            id: panelAppearTransition
            reversible: true
            NumberAnimation {
                target: contentArea
                properties: "posX,posY,width,height"
                duration: Style.panelsAnimationTime
                easing.type: Easing.InOutQuad
            }
        },
        Transition {
            from: "*"
            to: "fullscreen"
            reversible: true
            NumberAnimation {
                target: contentArea
                properties: "posX,posY,width,height"
                duration: Style.focusTransitionTime
                easing.type: Easing.OutQuad
            }
            PropertyAction {
                target: windowRect
                property: "z"
                value: Style.fullscreenZorder
            }
            NumberAnimation {
                target: windowRect
                property: "border.width"
                duration: Style.focusTransitionTime
                easing.type: Easing.OutQuad
            }
        }
    ]

    Behavior on border.color {
        ColorAnimation { duration: Style.focusTransitionTime }
    }
    Behavior on border.width {
        NumberAnimation { duration: Style.focusTransitionTime }
    }
    Behavior on opacity {
        NumberAnimation {
            duration: Style.panelsAnimationTime
            easing.type: Easing.InOutQuad
        }
    }
}
