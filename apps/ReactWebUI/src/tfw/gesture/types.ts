import { IBasicEvent } from "./basic-handler.types"

export interface IEvent extends IBasicEvent {
    preventDefault?: () => void;
    stopPropagation?: () => void;
    target: HTMLElement;
    x: number;
    y: number;
}

export interface IWheelEvent extends IEvent {
    delatX: number,
    deltaY: number,
    deltaZ: number
}
