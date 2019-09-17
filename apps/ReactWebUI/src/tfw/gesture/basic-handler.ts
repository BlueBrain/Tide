/**
 * BasicHandler(
 *     element: HTMLElement,
 *     handleDown: TBasicHandler,
 *     handleUp: TBasicHandler,
 *     handleMove: TBasicHandler
 * )
 *
 * Deals with three basic events : DOWN, UP and MOVE.
 * If the device has several input touches, we will return
 * only one event.
 *
 * A TBasicHandler is a synthetic event object:
 *   - x: X coordinate relative to the viewport, not including any scroll offset.
 *   - y: Y coordinate relative to the viewport, not including any scroll offset.
 *   - startX
 *   - startY
 *   - index: For multi-touch system. The first one is 0, the second is 1, etc.
 *   - buttons: 1 = left, 2 = right.
 *   - mouse: "mouse" | "touch" | "pen".
 *   - clear(): Call stopPropagation() and preventDefault() on this event.
 *
 */
import Finger from "./finger"
import { IBasicEvent } from "./basic-handler.types"

interface IMovingElement {
    // Coords relative to the element.
    x: number,
    y: number,
    index: number,
    target: HTMLElement,
    handleUp: (event: IBasicEvent) => void,
    handleMove: (event: IBasicEvent) => void
}

const movingElements: IMovingElement[] = [];

window.addEventListener("mousemove", (event: MouseEvent) => {
    for (const movingElem of movingElements) {
        if (event.cancelBubble) continue;
        const { target, handleMove, x, y, index } = movingElem;
        if (typeof handleMove !== 'function') continue;
        try {
            handleMove({
                x: event.clientX - x,
                y: event.clientY - y,
                index,
                event,
                pointer: "mouse",
                buttons: translateButtons(event),
                target,
                clear: createClear(event)
            })
        }
        catch (ex) {
            console.error(`[tfw.gesture.basic-handler] window.mousemove`, ex)
            console.error("    handler=", handleMove)
            console.error("    event=", event)
        }
    }
}, false);

window.addEventListener("mouseup", (event: MouseEvent) => {
    for (const movingElem of movingElements) {
        if (event.cancelBubble) continue;
        const { target, handleUp, x, y, index } = movingElem;
        if (typeof handleUp !== 'function') continue;
        try {
            handleUp({
                x: event.clientX - x,
                y: event.clientY - y,
                index,
                event,
                pointer: "mouse",
                buttons: translateButtons(event),
                target,
                clear: createClear(event)
            })
        }
        catch (ex) {
            console.error(`[tfw.gesture.basic-handler] window.mouseup`, ex)
            console.error("    handler=", handleUp)
            console.error("    event=", event)
        }
    }
    movingElements.splice(0, movingElements.length)
}, false);




type TTouchEventHandler = (evt: TouchEvent) => void;

type TMouseEventHandler = (evt: MouseEvent) => void;

type TBasicHandler = (evt: IBasicEvent) => void | undefined;

interface IDeviceHandlers {
    touchstart?: TTouchEventHandler;
    touchend?: TTouchEventHandler;
    touchmove?: TTouchEventHandler;
    mousedown?: TMouseEventHandler;
    mouseup?: TMouseEventHandler;
    mousemove?: TMouseEventHandler;
    mouseout?: TMouseEventHandler;
}

export default class BasicHandler {
    readonly element: HTMLElement;
    mouseType: string = "";
    mouseTypeTime: number = 0;
    deviceHandlers: IDeviceHandlers = {};
    fingers: Finger = new Finger();
    pressed: boolean = false;

    constructor(element: HTMLElement,
                handleDown: TBasicHandler,
                handleUp: TBasicHandler,
                handleMove: TBasicHandler) {
        this.element = element;
        attachDownEvent.call(this, handleDown, handleUp, handleMove);
    }

    /**
     * If you device can hold mouse and touch events, you will receive two events.
     * This function prevent it.
     *
     * @param   mouseType
     * @returns If `false`, we must ignore this event.
     */
    checkMouseType(mouseType: string): boolean {
        const now = Date.now();
        const delay = now - this.mouseTypeTime;
        this.mouseTypeTime = now;
        if (this.mouseType.length === 0 || delay > 500) {
            // If the user waits more than 500ms, he can change of mouse.
            this.mouseType = mouseType;
            return true;
        }
        return this.mouseType === mouseType;
    }

    detachEvents() {
        const element = this.element;
        const { touchstart, mousedown } = this.deviceHandlers;

        if (touchstart) element.removeEventListener("touchstart", touchstart, false);
        if (mousedown) element.removeEventListener("mousedown", mousedown, false);
    }
}


function attachDownEvent(this: BasicHandler,
                         handleDown: TBasicHandler,
                         handleUp: TBasicHandler,
                         handleMove: TBasicHandler) {
    attachDownEventTouch.call(this, handleDown, handleUp, handleMove);
    attachDownEventMouse.call(this, handleDown, handleUp, handleMove);
}


function attachDownEventTouch(this: BasicHandler,
                              handleDown: TBasicHandler,
                              handleUp: TBasicHandler,
                              handleMove: TBasicHandler) {
    const { element, deviceHandlers } = this;
    const handler = (event: TouchEvent) => {
        if (!this.checkMouseType("touch")) return;
        const rect = element.getBoundingClientRect();
        for (const touch of event.changedTouches) {
            const index = this.fingers.getIndex(touch.identifier)
            const x = touch.clientX - rect.left;
            const y = touch.clientY - rect.top;
            handleDown({
                x,
                y,
                index,
                event,
                buttons: 1,
                pointer: "touch",
                target: element,
                clear: createClear(event)
            });
            movingElements.push({
                handleUp, handleMove, index,
                target: element,
                x,
                y
            })
        }
    };
    deviceHandlers.touchstart = handler;
    element.addEventListener("touchstart", handler, false);
}


function attachDownEventMouse(this: BasicHandler,
                              handleDown: TBasicHandler,
                              handleUp: TBasicHandler,
                              handleMove: TBasicHandler) {
    const { element, deviceHandlers } = this;
    const handler = (event: MouseEvent) => {
        if (!this.checkMouseType("mouse")) return;
        const rect = element.getBoundingClientRect();
        this.pressed = true;
        const x = event.clientX - rect.left;
        const y = event.clientY - rect.top;
        if (typeof handleDown === 'function') {
            handleDown({
                x,
                y,
                index: 0,
                buttons: translateButtons(event),
                pointer: "mouse",
                event,
                target: element,
                clear: createClear(event)
            });
        }
        movingElements.push({
            handleUp, handleMove,
            index: 0,
            target: element,
            x: rect.left,
            y: rect.top
        })
    };
    deviceHandlers.mousedown = handler;
    element.addEventListener("mousedown", handler, false);
}


function createClear(evt: MouseEvent | TouchEvent) {
    return () => {
        evt.preventDefault();
        evt.stopPropagation();
    }
}

/**
 * Some mouses have only one button (on Mac, for instance).
 * So we will emulate the other buttons by looking at Ctrl and Meta keys.
 */
function translateButtons(event: MouseEvent): number {
    const { buttons, metaKey, ctrlKey } = event;
    if (buttons !== 1) return buttons;
    if (metaKey && !ctrlKey) return 2;
    if (!metaKey && ctrlKey) return 4;
    return buttons;
}
