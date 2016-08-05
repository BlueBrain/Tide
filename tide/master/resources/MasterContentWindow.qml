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
    property bool windowActive: contentwindow.mode === ContentWindow.STANDARD

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

    function toggleFullscreenMode() {
        if(contentwindow.isPanel)
            return
        if(contentwindow.fullscreen)
            displaygroup.exitFullscreen()
        else
            displaygroup.showFullscreen(contentwindow.id)
    }

    function toggleMode() {
        if(contentwindow.focused)
            displaygroup.unfocus(contentwindow.id)
        toggleFullscreenMode()
    }

    function moveWindow(delta) {
        contentwindow.controller.moveTo(Qt.point(contentwindow.x + delta.x,
                                                 contentwindow.y + delta.y))
    }

    function scaleWindow(center, pixelDelta) {
        contentwindow.controller.scale(center, pixelDelta)
    }

    focus: contentwindow.content.captureInteraction
    Keys.onPressed: {
        contentwindow.delegate.keyPress(event.key, event.modifiers, event.text)
        event.accepted = true;
    }
    Keys.onReleased: {
        contentwindow.delegate.keyRelease(event.key, event.modifiers, event.text)
        event.accepted = true;
    }

    virtualKeyboard.onLoaded: {
        // Process key events
        virtualKeyboard.item.hideKeyPressed.connect(function() {
            contentwindow.content.keyboard.visible = false
        })
        virtualKeyboard.item.keyPressed.connect(contentwindow.delegate.keyPress)
        virtualKeyboard.item.keyReleased.connect(contentwindow.delegate.keyRelease)
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

        onTouchStarted: displaygroup.moveContentWindowToFront(contentwindow.id)
        onTap: if(windowActive) { toggleControlsVisibility() }
        onTapAndHold: {
            if(contentwindow.isPanel) // force toggle
                contentwindow.controlsVisible = !contentwindow.controlsVisible
        }
        onDoubleTap: toggleFocusMode()

        onPanStarted: {
            if(windowActive && contentwindow.state === ContentWindow.NONE)
                contentwindow.state = ContentWindow.MOVING
        }
        onPan: {
            if(windowActive && contentwindow.state === ContentWindow.MOVING)
                moveWindow(delta)
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
            displaygroup.moveContentWindowToFront(contentwindow.id)
            if(contentActive)
                contentwindow.delegate.touchBegin(pos)
        }
        onTouchEnded: {
            if(contentActive)
                contentwindow.delegate.touchEnd(pos)
            contentwindow.content.captureInteraction = false
        }
        onTap: {
            if(contentActive)
               contentwindow.delegate.tap(pos)
            else if(windowActive)
                toggleControlsVisibility()
        }
        onDoubleTap: {
            if(!contentActive && !contentwindow.fullscreen)
                toggleFocusMode()
        }
        onTapAndHold: {
            if(contentActive)
                contentwindow.delegate.tapAndHold(pos, numPoints)
            else
                contentwindow.content.captureInteraction = true
        }

        onPanStarted: {
            if(!contentActive && windowActive && contentwindow.state === ContentWindow.NONE)
                contentwindow.state = ContentWindow.MOVING
        }
        onPan: {
            if(contentActive)
                contentwindow.delegate.pan(pos, Qt.point(delta.x, delta.y), numPoints)
            else if(windowActive && contentwindow.state === ContentWindow.MOVING)
                moveWindow(delta)
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
                contentwindow.delegate.pinch(pos, pixelDelta)
            else if(windowActive && contentwindow.state === ContentWindow.RESIZING)
                scaleWindow(pos, pixelDelta)
        }
        onPinchEnded: {
            if(!contentActive && windowActive && contentwindow.state === ContentWindow.RESIZING)
                contentwindow.state = ContentWindow.NONE
        }
        /** END shared gestures. */

        onSwipeLeft: contentwindow.delegate.swipeLeft()
        onSwipeRight: contentwindow.delegate.swipeRight()
        onSwipeUp: contentwindow.delegate.swipeUp()
        onSwipeDown: contentwindow.delegate.swipeDown()
    }

    previousButton.delegate: Triangle {
        MultitouchArea {
            anchors.fill: parent
            onTap: contentwindow.delegate.prevPage()
        }
    }
    nextButton.delegate: Triangle {
        MultitouchArea {
            anchors.fill: parent
            onTap: contentwindow.delegate.nextPage()
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
                    contentwindow.controller.adjustSizeOneToOne()
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
            onPan: contentwindow.controller.resizeRelative(delta)
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
        source: "qrc:///img/master/maximize.svg"
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        mousearea.onClicked: toggleMode()
        visible: !contentwindow.isPanel
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
            contentwindow.controller.resize(newSize)
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
