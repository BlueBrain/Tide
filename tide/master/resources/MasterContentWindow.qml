import QtQuick 2.0
import Tide 1.0
import TideMaster 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

BaseContentWindow {
    id: windowRect
    color: "#80000000"

    focusEffectEnabled: false // Not useful on the master window

    property bool contentActive: contentwindow.content.captureInteraction &&
                                 contentwindow.state === ContentWindow.NONE
    property bool windowActive: contentwindow.mode !== ContentWindow.FOCUSED

    function closeWindow() {
        groupcontroller.removeWindowLater(contentwindow.id)
    }

    function toggleSelected() {
        if(contentwindow.isPanel)
            return
        contentwindow.selected = !contentwindow.selected
    }

    function toggleFocusMode() {
        if(contentwindow.isPanel)
            return
        if(contentwindow.focused)
            groupcontroller.unfocus(contentwindow.id)
        else
            groupcontroller.focusSelected()
    }

    function toggleFullscreenMode() {
        if(contentwindow.isPanel)
            return
        if(contentwindow.fullscreen)
            groupcontroller.exitFullscreen()
        else
            groupcontroller.showFullscreen(contentwindow.id)
    }

    function scaleWindow(center, pixelDelta) {
        var sign = pixelDelta.x + pixelDelta.y > 0 ? 1.0 : -1.0;
        var delta = Math.sqrt(pixelDelta.x * pixelDelta.x +
                              pixelDelta.y * pixelDelta.y)
        controller.scale(center, sign * delta)
    }

    focus: contentwindow.content.captureInteraction
    Keys.onPressed: {
        contentcontroller.keyPress(event.key, event.modifiers, event.text)
        event.accepted = true;
    }
    Keys.onReleased: {
        contentcontroller.keyRelease(event.key, event.modifiers, event.text)
        event.accepted = true;
    }

    virtualKeyboard.onLoaded: {
        // Process key events
        virtualKeyboard.item.hideKeyPressed.connect(function() {
            contentwindow.content.keyboard.visible = false
        })
        virtualKeyboard.item.keyPressed.connect(contentcontroller.keyPress)
        virtualKeyboard.item.keyReleased.connect(contentcontroller.keyRelease)
        // Distribute keyboard state to the wall processes
        // use wrapper functions because the notifyChanged signals don't include the new value
        virtualKeyboard.item.shiftActiveChanged.connect(function() {
            contentwindow.content.keyboard.shift = virtualKeyboard.item.shiftActive
        })
        virtualKeyboard.item.symbolsActiveChanged.connect(function() {
            contentwindow.content.keyboard.symbols = virtualKeyboard.item.symbolsActive
        })
        virtualKeyboard.item.keyActivated.connect(contentwindow.content.keyboard.setActiveKeyId)
    }

    backgroundComponent: MultitouchArea {
        id: windowMoveAndResizeArea

        anchors.fill: parent
        referenceItem: windowRect.parent

        onTouchStarted: groupcontroller.moveWindowToFront(contentwindow.id)
        onTap: if(windowActive) { toggleSelected() }
        onTapAndHold: {
            if(contentwindow.isPanel) // force toggle
                contentwindow.selected = !contentwindow.selected
        }
        onDoubleTap: (numPoints > 1) ? toggleFocusMode() : toggleFullscreenMode()

        onPanStarted: {
            if(windowActive && contentwindow.state === ContentWindow.NONE)
                contentwindow.state = ContentWindow.MOVING
        }
        onPan: {
            if(windowActive && contentwindow.state === ContentWindow.MOVING)
                controller.moveBy(delta)
        }
        onPanEnded: {
            if(windowActive && contentwindow.state === ContentWindow.MOVING)
                contentwindow.state = ContentWindow.NONE
        }

        onPinchStarted: {
            if(windowActive && contentwindow.state === ContentWindow.NONE)
                contentwindow.state = ContentWindow.RESIZING
        }
        onPinch: {
            if(windowActive && contentwindow.state === ContentWindow.RESIZING)
                scaleWindow(pos, pixelDelta)
        }
        onPinchEnded: {
            if(windowActive && contentwindow.state === ContentWindow.RESIZING)
                contentwindow.state = ContentWindow.NONE
        }
    }

    contentComponent: MultitouchArea {
        id: contentInteractionArea

        // Explicit dimensions needed here. The item can't fill its parent
        // because the Loader has no size in this case (see BaseContentWindow).
        width: contentArea.width
        height: contentArea.height

        referenceItem: windowRect.parent

        /** Tap, pan and pinch gestures are used by either content or window. */
        onTouchStarted: {
            groupcontroller.moveWindowToFront(contentwindow.id)
            if(contentActive)
                contentcontroller.touchBegin(pos)
        }
        onTouchEnded: {
            if(contentActive)
                contentcontroller.touchEnd(pos)
            contentwindow.content.captureInteraction = false
        }
        onTouchPointAdded: {
            if(contentActive)
                contentcontroller.addTouchPoint(id, pos)
        }
        onTouchPointUpdated: {
            if(contentActive)
                contentcontroller.updateTouchPoint(id, pos)
        }
        onTouchPointRemoved: {
            if(contentActive)
                contentcontroller.removeTouchPoint(id, pos)
        }
        onTap: {
            if(contentActive)
               contentcontroller.tap(pos, numPoints)
            else if(windowActive)
                toggleSelected()
        }
        onDoubleTap: {
            if(contentActive)
                contentcontroller.doubleTap(pos, numPoints)
            else if(contentwindow.fullscreen)
                controller.toogleFullscreenMaxSize()
            else
                (numPoints > 1) ? toggleFocusMode() : toggleFullscreenMode()
        }
        onTapAndHold: {
            if(contentActive)
                contentcontroller.tapAndHold(pos, numPoints)
            else
                contentwindow.content.captureInteraction = true
        }

        onPanStarted: {
            if(!contentActive && windowActive && contentwindow.state === ContentWindow.NONE)
                contentwindow.state = ContentWindow.MOVING
        }
        onPan: {
            if(contentActive)
                contentcontroller.pan(pos, Qt.point(delta.x, delta.y), numPoints)
            else if(windowActive && contentwindow.state === ContentWindow.MOVING)
                controller.moveBy(delta)
        }
        onPanEnded: {
            if(!contentActive && windowActive && contentwindow.state === ContentWindow.MOVING)
                contentwindow.state = ContentWindow.NONE
        }

        onPinchStarted: {
            if(!contentActive && windowActive && contentwindow.state === ContentWindow.NONE)
                contentwindow.state = ContentWindow.RESIZING
        }
        onPinch: {
            if(contentActive)
                contentcontroller.pinch(pos, pixelDelta)
            else if(windowActive && contentwindow.state === ContentWindow.RESIZING)
                scaleWindow(pos, pixelDelta)
        }
        onPinchEnded: {
            if(!contentActive && windowActive && contentwindow.state === ContentWindow.RESIZING)
                contentwindow.state = ContentWindow.NONE
        }
        /** END shared gestures. */

        onSwipeLeft: contentcontroller.swipeLeft()
        onSwipeRight: contentcontroller.swipeRight()
        onSwipeUp: contentcontroller.swipeUp()
        onSwipeDown: contentcontroller.swipeDown()
    }

    previousButton.delegate: Triangle {
        MultitouchArea {
            anchors.fill: parent
            onTap: contentcontroller.prevPage()
        }
    }
    nextButton.delegate: Triangle {
        MultitouchArea {
            anchors.fill: parent
            onTap: contentcontroller.nextPage()
        }
    }

    windowControlsList.header: Column {
        CloseControlButton {
            MultitouchArea {
                anchors.fill: parent
                onTap: closeWindow()
            }
        }
        OneToOneControlButton {
            MultitouchArea {
                anchors.fill: parent
                onTap: {
                    controller.adjustSizeOneToOne()
                    contentwindow.content.resetZoom()
                }
            }
        }
        FullscreenControlButton {
            MultitouchArea {
                anchors.fill: parent
                onTap: toggleFullscreenMode()
            }
        }
        FocusControlButton {
            MultitouchArea {
                anchors.fill: parent
                onTap: toggleFocusMode()
            }
        }
    }

    windowControlsList.delegate: WindowControlsDelegate {
        MultitouchArea {
            anchors.fill: parent
            onTap: action.trigger()
        }
    }

    resizeCirclesDelegate: ResizeCircle {
        MultitouchArea {
            anchors.fill: parent
            referenceItem: windowRect.parent
            panThreshold: 10

            onTouchStarted: {
                contentwindow.activeHandle = parent.handle
                contentwindow.state = ContentWindow.RESIZING
            }
            onTapAndHold: {
                if(contentwindow.content.hasFixedAspectRatio)
                    contentwindow.resizePolicy = ContentWindow.ADJUST_CONTENT
                else
                    contentwindow.resizePolicy = ContentWindow.KEEP_ASPECT_RATIO
            }
            onPan: controller.resizeRelative(delta)
            onTouchEnded: {
                contentwindow.state = ContentWindow.NONE
                contentwindow.activeHandle = ContentWindow.NOHANDLE

                if(contentwindow.content.hasFixedAspectRatio)
                    contentwindow.resizePolicy = ContentWindow.KEEP_ASPECT_RATIO
                else
                    contentwindow.resizePolicy = ContentWindow.ADJUST_CONTENT
            }
        }
    }

    ContentWindowButton {
        source: "qrc:///img/master/close.svg"
        anchors.top: parent.top
        anchors.right: parent.right
        mousearea.onClicked: closeWindow()
    }

    ContentWindowButton {
        source: "qrc:///img/master/resize.svg"
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        visible: !contentwindow.isPanel &&
                 contentwindow.mode === ContentWindow.STANDARD

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
            controller.resize(newSize)
        }
        mousearea.onReleased: contentwindow.state = ContentWindow.NONE
    }

    Text {
        visible: !parent.titleBar.visible
        text: contentwindow.label
        font.pixelSize: 48
        width: Math.min(paintedWidth, parent.width)
        anchors.top: contentArea.top
        anchors.left: contentArea.left
        anchors.topMargin: 10
        anchors.leftMargin: 10
    }
}
