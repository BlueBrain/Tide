export interface IBasicEvent {
    // Coords relative to the element.
    x: number;
    y: number;
    startX?: number;
    startY?: number;
    index: number;
    event: MouseEvent | TouchEvent,
    buttons: number;
    pointer: "mouse" | "touch" | "pen";
    target: HTMLElement,
    clear: ()=>void;
}
