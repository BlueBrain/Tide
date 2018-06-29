import QtQuick 2.0
import Tide 1.0
import TideMaster 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

BaseWindow {
    id: windowRect
    color: "#80000000"

    focusEffectEnabled: false // Not useful on the master window

    property bool contentActive: window.content.captureInteraction
                                 && window.state === Window.NONE
    property bool windowActive: window.mode !== Window.FOCUSED

    function toggleSelected() {
        if (!window.isPanel) // it wouldn't be valid to check in the controller
            controller.toggleSelected()
    }

    function toggleFocusMode() {
        if (window.focused)
            groupcontroller.unfocus(window.id)
        else
            groupcontroller.focusSelected()
    }

    function toggleFullscreenMode() {
        groupcontroller.toggleFullscreen(window.id)
    }

    focus: window.content.captureInteraction
    Keys.onPressed: {
        contentcontroller.keyPress(event.key, event.modifiers, event.text)
        event.accepted = true
    }
    Keys.onReleased: {
        contentcontroller.keyRelease(event.key, event.modifiers, event.text)
        event.accepted = true
    }

    virtualKeyboard.onLoaded: {
        // Process key events
        virtualKeyboard.item.hideKeyPressed.connect(function () {
            window.content.keyboard.visible = false
        })
        virtualKeyboard.item.keyPressed.connect(contentcontroller.keyPress)
        virtualKeyboard.item.keyReleased.connect(contentcontroller.keyRelease)
        // Distribute keyboard state to the wall processes
        // use wrapper functions because the notifyChanged signals don't include the new value
        virtualKeyboard.item.shiftActiveChanged.connect(function () {
            window.content.keyboard.shift = virtualKeyboard.item.shiftActive
        })
        virtualKeyboard.item.symbolsActiveChanged.connect(function () {
            window.content.keyboard.symbols = virtualKeyboard.item.symbolsActive
        })
        virtualKeyboard.item.keyActivated.connect(
                    window.content.keyboard.setActiveKeyId)
    }

    backgroundComponent: MultitouchArea {
        id: windowMoveAndResizeArea

        anchors.fill: parent
        referenceItem: windowRect.parent

        onTouchStarted: groupcontroller.moveWindowToFront(window.id)
        onTap: if (windowActive) {
                   toggleSelected()
               }
        onTapAndHold: {
            if (window.isPanel)
                // force toggle
                controller.toggleSelected()
        }
        onDoubleTap: (numPoints > 1) ? toggleFocusMode(
                                           ) : toggleFullscreenMode()

        onPanStarted: {
            if (windowActive && window.state === Window.NONE)
                window.state = Window.MOVING
        }
        onPan: {
            if (windowActive && window.state === Window.MOVING)
                controller.moveBy(delta)
        }
        onPanEnded: {
            if (windowActive && window.state === Window.MOVING)
                window.state = Window.NONE
        }

        onPinchStarted: {
            if (windowActive && window.state === Window.NONE)
                window.state = Window.RESIZING
        }
        onPinch: {
            if (windowActive && window.state === Window.RESIZING)
                controller.scale(pos, pixelDelta)
        }
        onPinchEnded: {
            if (windowActive && window.state === Window.RESIZING)
                window.state = Window.NONE
        }
    }

    contentComponent: MultitouchArea {
        id: contentInteractionArea

        // Explicit dimensions needed here. The item can't fill its parent
        // because the Loader has no size in this case (see BaseWindow).
        width: contentArea.width
        height: contentArea.height

        referenceItem: windowRect.parent

        /** Tap, pan and pinch gestures are used by either content or window. */
        onTouchStarted: {
            groupcontroller.moveWindowToFront(window.id)
            if (contentActive)
                contentcontroller.touchBegin(pos)
        }
        onTouchEnded: {
            if (contentActive)
                contentcontroller.touchEnd(pos)
            window.content.captureInteraction = false
        }
        onTouchPointAdded: {
            if (contentActive)
                contentcontroller.addTouchPoint(id, pos)
        }
        onTouchPointUpdated: {
            if (contentActive)
                contentcontroller.updateTouchPoint(id, pos)
        }
        onTouchPointRemoved: {
            if (contentActive)
                contentcontroller.removeTouchPoint(id, pos)
        }
        onTap: {
            if (contentActive)
                contentcontroller.tap(pos, numPoints)
            else if (windowActive)
                toggleSelected()
        }
        onDoubleTap: {
            if (contentActive)
                contentcontroller.doubleTap(pos, numPoints)
            else if (window.fullscreen)
                controller.toogleFullscreenMaxSize()
            else
                (numPoints > 1) ? toggleFocusMode() : toggleFullscreenMode()
        }
        onTapAndHold: {
            if (contentActive)
                contentcontroller.tapAndHold(pos, numPoints)
            else
                window.content.captureInteraction = true
        }

        onPanStarted: {
            if (!contentActive && windowActive && window.state === Window.NONE)
                window.state = Window.MOVING
        }
        onPan: {
            if (contentActive)
                contentcontroller.pan(pos, Qt.point(delta.x, delta.y),
                                      numPoints)
            else if (windowActive && window.state === Window.MOVING
                     && numPoints === 1)
                controller.moveBy(delta)
        }
        onPanEnded: {
            if (!contentActive && windowActive
                    && window.state === Window.MOVING)
                window.state = Window.NONE
        }

        onPinchStarted: {
            if (!contentActive && windowActive && window.state === Window.NONE)
                window.state = Window.RESIZING
        }
        onPinch: {
            if (contentActive)
                contentcontroller.pinch(pos, pixelDelta)
            else if (windowActive && window.state === Window.RESIZING)
                controller.scale(pos, pixelDelta)
        }
        onPinchEnded: {
            if (!contentActive && windowActive
                    && window.state === Window.RESIZING)
                window.state = Window.NONE
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

    resizeCirclesDelegate: ResizeCircle {
        MultitouchArea {
            anchors.fill: parent
            referenceItem: windowRect.parent
            panThreshold: 10

            onTouchStarted: {
                window.activeHandle = parent.handle
                window.state = Window.RESIZING
            }
            onTapAndHold: {
                if (window.content.hasFixedAspectRatio)
                    window.resizePolicy = Window.ADJUST_CONTENT
                else
                    window.resizePolicy = Window.KEEP_ASPECT_RATIO
            }
            onPan: controller.resizeRelative(delta)
            onTouchEnded: {
                window.state = Window.NONE
                window.activeHandle = Window.NOHANDLE

                if (window.content.hasFixedAspectRatio)
                    window.resizePolicy = Window.KEEP_ASPECT_RATIO
                else
                    window.resizePolicy = Window.ADJUST_CONTENT
            }
        }
    }

    Text {
        visible: !parent.titleBar.visible
        text: window.content.title
        font.pixelSize: 48
        width: Math.min(paintedWidth, parent.width)
        anchors.top: contentArea.top
        anchors.left: contentArea.left
        anchors.topMargin: 10
        anchors.leftMargin: 10
    }
}
