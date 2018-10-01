import QtQuick 2.0
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
    property alias resizeCirclesDelegate: resizeCircles.delegate
    property alias titleBar: titleBar
    property alias virtualKeyboard: virtualKeyboard
    property alias rightButtonPageDelegate: rightButton.pageDelegate
    property alias leftButtonPageDelegate: leftButton.pageDelegate
    property alias rightButtonHandleDelegate: rightButton.handleDelegate
    property alias leftButtonHandleDelegate: leftButton.handleDelegate

    border.color: Style.windowBorderDefaultColor
    border.width: window.isPanel
                  || (options.showWindowBorders
                      && !isBackground) ? Style.windowBorderWidth : 0

    // The window header overlaps with the top window border (when visible)
    property real yOffset: Math.max(border.width, windowHeader.height)

    x: contentArea.posX - border.width
    y: contentArea.posY - yOffset
    width: contentArea.width + border.width * 2
    height: contentArea.height + border.width + yOffset // top + bottom padding
    z: isBackground ? Style.backgroundZOrder : window.isPanel ? Style.panelsZorder : 0

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
                visible: options.showWindowTitles && !window.focused
                         && !windowRect.isBackground && !window.isPanel
                width: parent.width
                height: Style.windowTitleHeight
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: Style.windowTitleFontSize / 2

                    FontLoader {
                        id: gothamBook
                        source: "qrc:/fonts/Gotham-Book.otf"
                        name: "qrc::gotham-book"
                    }

                    fontSizeMode: options.showFilePaths ? Text.Fit : Text.FixedSize
                    elide: options.showFilePaths ? Text.ElideLeft : Text.ElideRight
                    text: options.showFilePaths ? window.content.filePath : window.content.title
                    font {
                        family: "qrc::gotham-book"
                        pixelSize: Style.windowTitleFontSize
                    }
                }
            }
            Loader {
                id: controlBar
                width: parent.width
                visible: status == Loader.Ready
                active: (typeof window.content.playing !== "undefined")
                        && !windowRect.isBackground
                sourceComponent: MovieControls {
                }
            }
        }
    }

    Rectangle {
        id: contentArea
        property real posX: window.x
        property real posY: window.y
        width: window.width
        height: window.height

        anchors.bottom: parent.bottom
        anchors.margins: windowRect.border.width
        anchors.horizontalCenter: parent.horizontalCenter
        clip: true
        color: "transparent"

        Loader {
            id: contentBackgroundLoader
            // Note: this loader can't have a size, otherwise it breaks the
            // width/height bindings of loaded content in WallWindow.
        }

        Loader {
            id: virtualKeyboard
            source: "qrc:/virtualkeyboard/InputPanel.qml"
            property bool hasKeyboard: window.content.keyboard !== null
            active: hasKeyboard && window.content.keyboard.visible
            width: Math.min(Style.keyboardMaxSizePx,
                            Style.keyboardRelSize * parent.width)
            height: 0.25 * width
            anchors.horizontalCenter: parent.horizontalCenter
            y: hasKeyboard
               && window.content.keyboard.visible ? Math.min(
                                                        0.5 * parent.height,
                                                        parent.height - height) : parent.height
            opacity: hasKeyboard && window.content.keyboard.visible ? 1.0 : 0.0
            visible: opacity > 0.0
            Behavior on y {
                PropertyAnimation {
                    easing.type: Easing.InOutQuad
                    duration: 150
                }
            }
            Behavior on opacity {
                PropertyAnimation {
                    easing.type: Easing.InOutQuad
                    duration: 150
                }
            }
            onStatusChanged: {
                if (status == Loader.Error)
                    source = "qrc:/qml/core/MissingVirtualKeyboard.qml"
                else if (status == Loader.Ready)
                    active = true // Keep the keyboard loaded when hidding it
            }
        }
        Behavior on posX {
            id: transitionBehaviour
            enabled: window.focused || (window.fullscreen
                                        && window.state === Window.NONE)
            PropertyAnimation {
                duration: Style.focusTransitionTime
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on posY {
            enabled: transitionBehaviour.enabled
            PropertyAnimation {
                duration: Style.focusTransitionTime
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on width {
            enabled: transitionBehaviour.enabled
            PropertyAnimation {
                id: widthAnimation
                duration: Style.focusTransitionTime
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on height {
            enabled: transitionBehaviour.enabled
            PropertyAnimation {
                id: heightAnimation
                duration: Style.focusTransitionTime
                easing.type: Easing.InOutQuad
            }
        }
    }

    WindowSideButton {
        id: leftButton
        anchors.left: contentArea.left
        anchors.verticalCenter: contentArea.verticalCenter
    }

    WindowSideButton {
        id: rightButton
        flipRight: true
        anchors.right: contentArea.right
        anchors.verticalCenter: contentArea.verticalCenter
    }

    WindowControls {
        id: windowControls
        color: windowRect.border.color
    }

    ResizeCircles {
        id: resizeCircles
    }

    states: [
        State {
            name: "fullscreen"
            when: window.fullscreen
            PropertyChanges {
                target: contentArea
                posX: window.fullscreenCoordinates.x
                posY: window.fullscreenCoordinates.y
                width: window.fullscreenCoordinates.width
                height: window.fullscreenCoordinates.height
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
            when: window.focused
            PropertyChanges {
                target: contentArea
                posX: window.focusedCoordinates.x
                posY: window.focusedCoordinates.y
                width: window.focusedCoordinates.width
                height: window.focusedCoordinates.height
            }
            PropertyChanges {
                target: windowRect
                border.color: Style.windowBorderFocusedColor
                z: Style.focusZorder
            }
        },
        State {
            name: "selected"
            when: window.selected
            PropertyChanges {
                target: windowRect
                border.color: Style.windowBorderSelectedColor
            }
        },
        State {
            name: "moving"
            when: window.state === Window.MOVING
            PropertyChanges {
                target: windowRect
                border.color: Style.windowBorderMovingColor
            }
        },
        State {
            name: "resizing"
            when: window.state === Window.RESIZING
            PropertyChanges {
                target: windowRect
                border.color: Style.windowBorderResizingColor
            }
        },
        State {
            name: "hidden"
            when: window.state === Window.HIDDEN
            PropertyChanges {
                target: contentArea
                posX: -window.width
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
            to: "selected," // selected AND default state ""
            id: unfocusTransition
            SequentialAnimation {
                // Slide behind other focused windows for the transition
                PropertyAction {
                    target: windowRect
                    property: "z"
                    value: Style.unfocusZorder
                }
                ParallelAnimation {
                    ColorAnimation {
                        duration: Style.focusTransitionTime
                    }
                    PropertyAnimation {
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
            PropertyAnimation {
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
            PropertyAnimation {
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
            PropertyAnimation {
                target: windowRect
                property: "border.width"
                duration: Style.focusTransitionTime
                easing.type: Easing.OutQuad
            }
        }
    ]

    Behavior on border.color {
        ColorAnimation {
            duration: Style.focusTransitionTime
        }
    }
    Behavior on border.width {
        PropertyAnimation {
            duration: Style.focusTransitionTime
        }
    }
    Behavior on opacity {
        PropertyAnimation {
            duration: Style.panelsAnimationTime
            easing.type: Easing.InOutQuad
        }
    }
}
