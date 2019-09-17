import Gesture from "../gesture"
import Theme from "../theme"
import castString from "../converter/string"
import castBoolean from "../converter/boolean"
import castFunction from "../converter/function"
import "./touchable.css"

interface IArgs {
    element?: HTMLElement;
    enabled?: boolean;
    color?: string;
    onTap?: () => void;
}

interface IRect {
    left: number;
    top: number;
    width: number;
    height: number;
}

export default class Touchable {
    _element: HTMLElement | undefined;
    _enabled: boolean = true;
    color: string = "#000";
    accent: boolean = false;
    onTap: () => void;

    constructor(args: IArgs) {
        this.element = args.element;
        this.enabled = castBoolean(args.enabled, true);
        this.color = castString(args.color, "");
        this.onTap = castFunction(args.onTap);
    }

    fire() {
        if (!this.enabled) return;

        const handler = this.onTap;
        if (typeof handler === 'function') {
            handler();
        }
    }

    get enabled() { return this._enabled; }
    set enabled(value: boolean) { this._enabled = value; }

    get element() { return this._element; }
    set element(element: HTMLElement | undefined) {
        const that = this;
        if (typeof element === 'undefined') return;
        Gesture(element).on({
            keydown(evt: any) {
                if (!that.enabled) return;

                const key = evt.key;
                if (key === " " || key === "Enter") {
                    evt.preventDefault();
                    evt.stopPropagation();
                    that.fire();
                }
            },

            down(evt) {
                if (!that.enabled) return;
            },

            up(evt) {
                if (!that.removeTouchDisk) return;
                that.removeTouchDisk();
                delete that.removeTouchDisk;
            },

            tap(evt) {
                if (!that.enabled) return;
                that.fire();

                const rect = element.getBoundingClientRect();
                const style = window.getComputedStyle(element);
                const screen = createScreen(rect);
                screen.style.borderRadius = style.getPropertyValue("border-radius");
                const x = evt.x || 0;
                const y = evt.y || 0;
                const w = Math.max(x, rect.width - x);
                const h = Math.max(y, rect.height - y);
                const radius = Math.ceil(1.1 * Math.sqrt(w * w + h * h));
                const ripple = document.createElement("div");
                screen.appendChild(ripple);
                ripple.style.left = `${x - radius}px`;
                ripple.style.top = `${y - radius}px`;
                ripple.style.width = `${2 * radius}px`;
                ripple.style.height = `${2 * radius}px`;
                if (that.color.length > 0) {
                    ripple.style.background = that.color;
                } else {
                    ripple.style.background = Theme.isDark() ? "#fff" : "#000";
                }
                window.setTimeout(() => ripple.className = "open");
                window.setTimeout(() => document.body.removeChild(screen), 500);
            }
        })
    }
}


function createScreen(rect: IRect) {
    const div = document.createElement("div");
    div.className = "tfw-behavior-touchable";
    div.style.left = `${rect.left}px`;
    div.style.top = `${rect.top}px`;
    div.style.width = `${rect.width}px`;
    div.style.height = `${rect.height}px`;
    document.body.appendChild(div);
    return div;
}
