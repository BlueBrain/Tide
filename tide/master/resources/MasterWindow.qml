import QtQuick 2.0
import Tide 1.0
import TideMaster 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

BaseWindow {
    id: windowRect
    color: "#80000000"

    focusEffectEnabled: false // Not useful on the master window

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

        onTouchStarted: touchcontroller.onTouchStarted()
        onTap: touchcontroller.onTap()
        onTapAndHold: touchcontroller.onTapAndHold()
        onDoubleTap: touchcontroller.onDoubleTap(numPoints)

        onPanStarted: touchcontroller.onPanStarted()
        onPan: touchcontroller.onPan(pos, delta, numPoints)
        onPanEnded: touchcontroller.onPanEnded()

        onPinchStarted: touchcontroller.onPinchStarted()
        onPinch: touchcontroller.onPinch(pos, pixelDelta)
        onPinchEnded: touchcontroller.onPinchEnded()

        onSwipeLeft: contentcontroller.swipeLeft()
        onSwipeRight: contentcontroller.swipeRight()
        onSwipeUp: contentcontroller.swipeUp()
        onSwipeDown: contentcontroller.swipeDown()
    }

    contentComponent: MultitouchArea {
        id: contentInteractionArea

        // Explicit dimensions needed here. The item can't fill its parent
        // because the Loader has no size in this case (see BaseWindow).
        width: contentArea.width
        height: contentArea.height
        referenceItem: contentArea

        enabled: window.content.captureInteraction
                 && window.state === Window.NONE

        onTouchStarted: {
            groupcontroller.moveWindowToFront(window.id)
            contentcontroller.touchBegin(pos)
        }
        onTouchEnded: contentcontroller.touchEnd(pos)

        onTouchPointAdded: contentcontroller.addTouchPoint(id, pos)
        onTouchPointUpdated: contentcontroller.updateTouchPoint(id, pos)
        onTouchPointRemoved: contentcontroller.removeTouchPoint(id, pos)

        onTap: contentcontroller.tap(pos, numPoints)
        onDoubleTap: contentcontroller.doubleTap(pos, numPoints)
        onTapAndHold: contentcontroller.tapAndHold(pos, numPoints)

        onPan: contentcontroller.pan(pos, delta, numPoints)
        onPinch: contentcontroller.pinch(pos, pixelDelta)

        onSwipeLeft: contentcontroller.swipeLeft()
        onSwipeRight: contentcontroller.swipeRight()
        onSwipeUp: contentcontroller.swipeUp()
        onSwipeDown: contentcontroller.swipeDown()
    }

    resizeCirclesDelegate: ResizeCircle {
        MultitouchArea {
            anchors.fill: parent
            referenceItem: windowRect.parent
            panThreshold: 10

            onTouchStarted: controller.startResizing(parent.handle)
            onTapAndHold: controller.toggleResizeMode()
            onPan: controller.resizeRelative(delta)
            onTouchEnded: controller.stopResizing()
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
