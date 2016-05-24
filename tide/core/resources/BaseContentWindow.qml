import QtQuick 2.0
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

    Item {
        id: contentArea
        anchors.bottom: parent.bottom
        anchors.bottomMargin: windowRect.border.width
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - windowRect.widthOffset
        height: parent.height - windowRect.heightOffset
        clip: true

        Loader {
            id: contentBackgroundLoader
            anchors.fill: parent
        }

        SideButton {
            id: previousButton
            color: windowRect.border.color
            delegate: Triangle {
            }
            delegateOverflow: windowRect.border.width
            visible: (contentwindow.controlsVisible || contentwindow.focused) &&
                     contentwindow.content.page !== undefined &&
                     contentwindow.content.page > 0
        }

        SideButton {
            id: nextButton
            color: windowRect.border.color
            delegate: Triangle {
            }
            delegateOverflow: windowRect.border.width
            flipRight: true
            visible: (contentwindow.controlsVisible || contentwindow.focused) &&
                     contentwindow.content.page !== undefined &&
                     contentwindow.content.page < contentwindow.content.pageCount - 1
        }
    }

    WindowControls {
        id: windowControls
    }

    ResizeCircles {
        id: resizeCircles
    }

    states: [
        State {
            name: "focused"
            when: contentwindow.focused
            PropertyChanges {
                target: windowRect
                x: contentwindow.focusedCoordinates.x - xOffset
                y: contentwindow.focusedCoordinates.y - yOffset
                width: contentwindow.focusedCoordinates.width + widthOffset
                height: contentwindow.focusedCoordinates.height + heightOffset
                border.color: Style.windowBorderSelectedColor
                z: Style.focusZorder
            }
        },
        State {
            name: "selected"
            when: contentwindow.state === ContentWindow.SELECTED
            PropertyChanges {
                target: windowRect
                border.color: Style.windowBorderSelectedColor
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
            when: contentwindow.state === ContentWindow.HIDDEN ||
                  (contentwindow.isPanel && !completed())
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
