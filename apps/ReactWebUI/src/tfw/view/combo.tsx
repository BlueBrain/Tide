import * as React from "react"
import "./combo.css"
import castString from "../converter/string"
import castBoolean from "../converter/boolean"
import castStringArray from "../converter/string-array"
import Touchable from "../behavior/touchable"
import Icon from "./icon"
import Label from "./label"
import Gesture from "../gesture"
import EscapeHandler from "../escape-handler"

interface IComboProps {
    label?: string;
    value?: string;
    keys?: string[];
    wide?: boolean;
    onChange?: (value: string) => void;
    children: React.ReactElement<any>[];
}

export default class Combo extends React.Component<IComboProps, {}> {
    readonly touchable: Touchable;

    ref = React.createRef<HTMLButtonElement>()
    list = React.createRef<HTMLDivElement>()
    button = React.createRef<HTMLDivElement>()

    bigList: HTMLElement | undefined;

    keys: string[] = []
    selectedKey: string = ""
    selectedIndex: number = -1

    constructor(props: IComboProps) {
        super(props);
        this.handleClick = this.handleClick.bind(this);
        this.touchable = new Touchable({ onTap: this.handleClick });
    }

    handleClick() {
        this.expand();
    }

    /**
     * If there are only two items, we don't open the list: we just swap them.
     */
    _swapItems() {
        // With two items, clicking will immediatly switch to the next one.
        for (const key of this.keys) {
            if (this.props.value !== key) {
                const slot = this.props.onChange;
                if (typeof slot === 'function') {
                    slot(key);
                }
                return;
            }
        }
    }

    _computeDimensions(list: HTMLElement, button: HTMLElement): { left: number, top: number, width: number, height: number } {
        let { left, top, height } = list.getBoundingClientRect();
        const { width } = button.getBoundingClientRect();
        height = Math.min(height, window.innerHeight - 32);
        if (top + height > window.innerHeight) {
            top -= 32 * Math.ceil(window.innerHeight - top - height / 32);
        }
        if (top < 0) {
            top += 32 * Math.ceil(-top / 32);
        }
        return { left, top, width, height };
    }

    _showList(list: HTMLElement, button: HTMLElement, left: number, top: number, width: number, height: number): { screen: HTMLElement, bigList: HTMLElement } {
        const screen = document.createElement("div");
        screen.className = "tfw-view-combo-screen";
        const bigList = document.createElement("div");
        bigList.className = "thm-ele-nav thm-bg3";
        bigList.innerHTML = list.innerHTML;

        bigList.style.top = `${top}px`;
        bigList.style.left = `${left}px`;
        bigList.style.width = `${width}px`;
        bigList.style.height = `${height}px`;
        screen.appendChild(bigList);
        document.body.appendChild(screen);

        this.bigList = bigList;
        return { screen, bigList };
    }

    _ensureSelectionVisible() {
        let index = 0;
        const bigList: HTMLElement | undefined = this.bigList;
        if (!bigList) return;
        const items = bigList.querySelectorAll("div.item");
        for (const key of this.keys) {
            const item = items[index];
            if (key === this.selectedKey) {
                const rect = bigList.getBoundingClientRect();
                bigList.scrollTop = (32 * index) - rect.height / 2;
                this.selectedIndex = index;
                item.classList.add("thm-bgSL");
            } else {
                item.classList.remove("thm-bgSL");
            }
            index++;
        }
    }

    _handleKeyboard(event: React.KeyboardEvent) {
        switch (event.key) {
            case "ArrowDown":
                this.selectedIndex++;
                this.selectedKey = this.keys[this.selectedIndex % this.keys.length];
                this._ensureSelectionVisible();
                event.preventDefault();
                event.stopPropagation();
                return;
            case "ArrowUp":
                this.selectedIndex = (this.selectedIndex + this.keys.length - 1) % this.keys.length;
                this.selectedKey = this.keys[this.selectedIndex];
                this._ensureSelectionVisible();
                event.preventDefault();
                event.stopPropagation();
                return;
            case "Enter":
            case " ":
                const slot = this.props.onChange;
                if (typeof slot === 'function') {
                    slot(this.selectedKey);
                }
                EscapeHandler.fire();
                event.preventDefault();
                event.stopPropagation();
                return;
        }
    }

    expand() {
        if (this.keys.length < 3) {
            this._swapItems();
            return;
        }

        const list = this.list.current;
        const button = this.button.current;
        if (!list || !button) return;

        const { left, top, width, height } = this._computeDimensions(list, button);
        const { screen, bigList } = this._showList(list, button, left, top, width, height);
        this.selectedKey = this.props.value || "";
        window.setTimeout(() => this._ensureSelectionVisible(bigList), 10);

        const handleKeyboard = this._handleKeyboard.bind(this);
        document.addEventListener("keydown", handleKeyboard, true);
        EscapeHandler.add(() => {
            document.body.removeChild(screen)
            document.removeEventListener("keydown", handleKeyboard, true);
        });
        Gesture(screen).on({ tap: () => {
            EscapeHandler.fire()
        }});
        Gesture(bigList).on({
            tap: evt => {
                if (!evt || typeof evt.y === 'undefined') return;
                const scroll = bigList.scrollTop;
                const index = Math.floor((evt.y + scroll) / 32);
                const value = this.keys[index];
                EscapeHandler.fire()
                requestAnimationFrame(() => {
                    if (typeof this.props.onChange === 'function') {
                        this.props.onChange(value)
                    }
                });
            }
        });
    }

    componentDidMount() {
        const element = this.ref.current;
        if (!element) return;
        this.touchable.element = element;
    }

    render() {
        const p = this.props;
        const children = p.children;
        const label = castString(p.label, "").trim();
        const wide = castBoolean(p.wide, false);
        const keys = ensureGoodKeys(p.keys, children);
        const value = castString(p.value, keys[0]);
        const classes = ["tfw-view-combo"];
        const items = children.map(item => {
            const key = item.key;
            return (<div className="item" key={key} > {item} </div>);
        });

        if (wide) classes.push("wide");

        this.keys = keys;

        const index = getIndex(keys, value);

        return (
            <button ref={this.ref} className={classes.join(" ")}>
                {label.length > 0 ? <Label label={label}/> : null}
                <div ref={this.button} className="button thm-bg3 thm-ele-button">
                    <div className="list-container"
                        style={{
                            transform: `translateY(-${32 * index}px)`
                        }}>
                        <div ref={this.list} className="tfw-view-combo-list" >{items}</div>
                    </div>
                    <div className="icon" >
                        <Icon size="24px" content="down" />
                    </div>
                </div>
            </button>
        );
    }
}

function getIndex(keys: string[], value: string) {
    const index = keys.indexOf(value);
    return Math.max(0, index);
}

/**
 * Keys must be non empty strings. If a key is not defined, it will take its index (stringified) as value.
 * And if a "key" its defined on a child, it will take precedence on anything else.
 *
* @param  {string[]} keys
* @param  {ReactElement[]} children
          *
* @return {string[]}
*/
function ensureGoodKeys(keys: string[] | undefined, children: React.ReactElement<any>[]): string[] {
    const goodKeys: string[] = castStringArray(keys, []);
    const minimalLength: number = children.length;

    while (goodKeys.length < minimalLength) {
        goodKeys.push(`${goodKeys.length}`);
    }
    for (let k = 0; k < goodKeys.length; k++) {
        if (goodKeys[k].trim().length === 0) {
            goodKeys[k] = `${k}`;
        }
    }
    children.forEach((child, index) => {
        const key = child.key;
        if (typeof key === "string") {
            goodKeys[index] = key;
        } else {
            child.key = goodKeys[index];
        }
    });

    return goodKeys;
}


function onTap(elem: HTMLElement, handler: () => void) {
    const slot = (event) => {
        event.preventDefault();
        event.stopPropagation();
        elem.removeEventListener("mousedown", slot);
        elem.removeEventListener("touchdown", slot);
        handler();
    };
    elem.addEventListener("mousedown", slot, false);
    elem.addEventListener("touchdown", slot, false);
}
