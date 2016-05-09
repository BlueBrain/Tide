import QtQuick 2.0
import Tide 1.0
import TideMaster 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

BaseContentWindow {
    id: windowRect
    color: "#80000000"

    function closeWindow() {
        displaygroup.removeWindowLater(contentwindow.id)
    }

    function toggleControlsVisibility() {
        if(contentwindow.isPanel)
            return
        contentwindow.controlsVisible = !contentwindow.controlsVisible
    }

    function toggleFocusMode() {
        if(contentwindow.isPanel)
            return
        if(contentwindow.focused)
            displaygroup.unfocus(contentwindow.id)
        else
            displaygroup.focus(contentwindow.id)
    }

    focus: contentwindow.state === ContentWindow.SELECTED
    Keys.onPressed: {
        contentwindow.delegate.keyPress(event.key, event.modifiers, event.text)
        event.accepted = true;
    }
    Keys.onReleased: {
        contentwindow.delegate.keyRelease(event.key, event.modifiers, event.text)
        event.accepted = true;
    }

    MultitouchArea
    {
        id: windowMoveAndResizeArea

        anchors.fill: parent
        referenceItem: windowRect.parent

        visible: contentwindow.state !== ContentWindow.SELECTED &&
                 contentwindow.border === ContentWindow.NOBORDER

        onTouchStarted: displaygroup.moveContentWindowToFront(contentwindow.id)
        onTouchEnded: contentwindow.state = ContentWindow.NONE

        onTap: toggleControlsVisibility()
        onDoubleTap: toggleFocusMode()

        onPanStarted: contentwindow.state = ContentWindow.MOVING
        onPan: contentwindow.controller.moveTo(Qt.point(contentwindow.x + delta.x,
                                                        contentwindow.y + delta.y))
        onPanEnded: contentwindow.state = ContentWindow.NONE

        onPinch: {
            contentwindow.state = ContentWindow.RESIZING
            contentwindow.controller.scale(pos, pixelDelta)
        }
    }

    MultitouchArea {
        id: contentInteractionArea
        visible: contentwindow.state === ContentWindow.SELECTED &&
                 contentwindow.border === ContentWindow.NOBORDER

        anchors.bottom: parent.bottom
        anchors.bottomMargin: windowRect.border.width
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - windowRect.widthOffset
        height: parent.height - windowRect.heightOffset

        referenceItem: windowRect.parent

        onDoubleTap: toggleFocusMode()
        onTouchStarted: {
            displaygroup.moveContentWindowToFront(contentwindow.id)
            contentwindow.delegate.touchBegin(pos)
        }
        onTouchEnded: contentwindow.delegate.touchEnd(pos)
        onTap: contentwindow.delegate.tap(pos)
        onTapAndHold: contentwindow.delegate.tapAndHold(pos)
        onPan: contentwindow.delegate.pan(pos, Qt.point(delta.x, delta.y))
        onPinch: contentwindow.delegate.pinch(pos, pixelDelta)
        onSwipeLeft: contentwindow.delegate.swipeLeft()
        onSwipeRight: contentwindow.delegate.swipeRight()
        onSwipeUp: contentwindow.delegate.swipeUp()
        onSwipeDown: contentwindow.delegate.swipeDown()
    }

    ContentWindowButton {
        source: "qrc:///img/master/close.svg"
        anchors.top: parent.top
        anchors.right: parent.right
        mousearea.onClicked: closeWindow()
    }

    ContentWindowButton {
        source: "qrc:///img/master/maximize.svg"
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        mousearea.onClicked: toggleFocusMode()
        visible: !contentwindow.isPanel
    }

    ContentWindowButton {
        source: "qrc:///img/master/resize.svg"
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        visible: !contentwindow.isPanel && !contentwindow.focused

        property variant startMousePos
        property variant startSize
        mousearea.onPressed: {
            startMousePos = Qt.point(mouse.x, mouse.y)
            startSize = Qt.size(contentwindow.width, contentwindow.height)
            contentwindow.state = ContentWindow.RESIZING
        }
        mousearea.onPositionChanged: {
            if(!mousearea.pressed)
                return
            var newSize = Qt.size(mouse.x - startMousePos.x + startSize.width,
                                  mouse.y - startMousePos.y + startSize.height)
            contentwindow.controller.resize(newSize)
        }
        mousearea.onReleased: contentwindow.state = ContentWindow.NONE
    }

    ResizeCircles {
        resizeCirclesDelegate: touchBorderDelegate
        Component {
            id: touchBorderDelegate
            ResizeCircle {
                MultitouchArea
                {
                    anchors.fill: parent
                    referenceItem: windowRect.parent
                    panThreshold: 0

                    onPanStarted: {
                        contentwindow.border = parent.border
                        contentwindow.state = ContentWindow.RESIZING
                    }

                    onPan: contentwindow.controller.resizeRelative(delta)

                    onPanEnded: {
                        contentwindow.border = ContentWindow.NOBORDER
                        contentwindow.state = ContentWindow.NONE
                    }
                }
            }
        }
    }

    WindowControls {
        listview.delegate: buttonDelegate
        listview.header: fixedButtonsDelegate

        Component {
            id: fixedButtonsDelegate
            Column {
                CloseControlButton {
                    MultitouchArea
                    {
                        anchors.fill: parent
                        onTap: closeWindow()
                    }
                }
                OneToOneControlButton {
                    MultitouchArea
                    {
                        anchors.fill: parent
                        onTap: contentwindow.controller.adjustSizeOneToOne()
                    }
                }
                FocusControlButton {
                    MultitouchArea
                    {
                        anchors.fill: parent
                        onTap: toggleFocusMode()
                    }
                }
            }
        }

        Component {
            id: buttonDelegate
            WindowControlsDelegate {
                MultitouchArea
                {
                    anchors.fill: parent
                    onTap: action.trigger()
                }
            }
        }
    }

    Text {
        visible: !parent.titleBar.visible
        text: contentwindow.label
        font.pixelSize: 48
        width: Math.min(paintedWidth, parent.width)
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 10
        anchors.leftMargin: 10
    }
}
