import QtQuick 2.0
import QtGraphicalEffects 1.0
import Tide 1.0
import "style.js" as Style

Rectangle {
    id: windowRect

    property alias titleBar: titleBar
    property bool isBackground: false
    property bool animating: widthAnimation.running || heightAnimation.running
                             || unfocusTransition.running

    property real widthOffset: 2 * border.width
    property real heightOffset: border.width + (titleBar.visible ? titleBar.height : border.width)
    property real xOffset: border.width
    property real yOffset: titleBar.visible ? titleBar.height : border.width

    property alias backgroundComponent: backgroundLoader.sourceComponent
    property alias contentComponent: contentBackgroundLoader.sourceComponent
    property alias contentArea: contentArea
    property alias windowControlsList: windowControls.listview
    property alias resizeCirclesDelegate: resizeCircles.delegate
    property alias previousButton: previousButton
    property alias nextButton: nextButton
    property alias focusEffectEnabled: focusEffect.visible

    border.color: Style.windowBorderDefaultColor
    border.width: options.showWindowBorders && !isBackground ? Style.windowBorderWidth : 0

    x: contentwindow.x - xOffset
    y: contentwindow.y - yOffset
    z: isBackground ? Style.backgroundZOrder : 0
    width: contentwindow.width + widthOffset
    height: contentwindow.height + heightOffset

    Rectangle {
        id: titleBar
        visible: displaygroup.showWindowTitles && !windowRect.isBackground
                 && !contentwindow.isPanel
        width: parent.width
        height: Style.windowTitleHeight
        color: parent.border.color

        Text {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: Style.windowTitleFontSize / 4

            FontLoader { id: gothamBook; source: "qrc:/fonts/Gotham-Book.otf"; name: "qrc::gotham-book" }

            elide: Text.ElideRight
            text: contentwindow.label
            font { family: "qrc::gotham-book"; pixelSize: Style.windowTitleFontSize }
        }
    }

    Loader {
        id: backgroundLoader
        anchors.fill: parent
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
        anchors.bottom: parent.bottom
        anchors.bottomMargin: windowRect.border.width
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - windowRect.widthOffset
        height: parent.height - windowRect.heightOffset
        clip: true
        color: "transparent"

        Loader {
            id: contentBackgroundLoader
            // Note: this loader can't have a size, otherwise it breaks the
            // width/height bindings of loaded content in WallContentWindow.
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
    }

    ResizeCircles {
        id: resizeCircles
    }

    states: [
        State {
            name: "fullscreen"
            when: contentwindow.fullscreen
            PropertyChanges {
                target: windowRect
                x: contentwindow.fullscreenCoordinates.x
                y: contentwindow.fullscreenCoordinates.y
                width: contentwindow.fullscreenCoordinates.width
                height: contentwindow.fullscreenCoordinates.height
                border.width: 0
                z: Style.fullscreenZorder
            }
            PropertyChanges {
                target: titleBar
                visible: false
            }
        },
        State {
            name: "focused"
            when: contentwindow.focused
            PropertyChanges {
                target: windowRect
                x: contentwindow.focusedCoordinates.x - xOffset
                y: contentwindow.focusedCoordinates.y - yOffset
                width: contentwindow.focusedCoordinates.width + widthOffset
                height: contentwindow.focusedCoordinates.height + heightOffset
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
                target: windowRect
                opacity: 0
                x: -contentwindow.width
                y: 0.5 * displaygroup.height
                width: 0
                height: 0
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
                        target: windowRect
                        properties: "x,y,height,width"
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
                target: windowRect
                properties: "x,y,height,width"
                duration: Style.panelsAnimationTime
                easing.type: Easing.InOutQuad
            }
        },
        Transition {
            from: "*"
            to: "fullscreen"
            reversible: true
            NumberAnimation {
                properties: "x,y,width,height"
                duration: Style.focusTransitionTime
                easing.type: Easing.OutQuad
            }
            PropertyAction {
                target: windowRect
                property: "z"
                value: Style.fullscreenZorder
            }
        }
    ]

    Behavior on border.color {
        ColorAnimation { duration: Style.focusTransitionTime }
    }
    Behavior on x {
        enabled: contentwindow.focused
        NumberAnimation {
            duration: Style.focusTransitionTime
            easing.type: Easing.InOutQuad
        }
    }
    Behavior on y {
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
    Behavior on opacity {
        NumberAnimation {
            id: opacityAnimation
            duration: Style.panelsAnimationTime
            easing.type: Easing.InOutQuad
        }
    }
}
