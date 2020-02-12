import * as React from "react"
import castInteger from "../../converter/integer"
import castBoolean from "../../converter/boolean"
import castString from "../../converter/string"
import castUnit from "../../converter/unit"
import Debouncer from "../../debouncer"
import Label from "../label"

import "./text-area.css"

import Intl from "../../intl"
const _ = Intl.make(require("./text-area.yaml"));

interface IStringSlot {
    (value: string): void;
}

interface ITextAreaProps {
    cols?: number;
    rows?: number;
    label?: string;
    value?: string;
    wide?: boolean;
    delay?: number;
    width?: string;
    onChange?: IStringSlot
}

export default class TextArea extends React.Component<ITextAreaProps, {}> {
    // The onChange event is not fired at each kew stroke.
    private lastEmittedValue: string = `kĴqwekb-${Math.random()}${Date.now()}-wcçkkj`
    readonly input: React.RefObject<HTMLTextAreaElement>;
    value: string = "";

    constructor(props: ITextAreaProps) {
        super(props);
        this.state = {};
        this.onFocus = this.onFocus.bind(this);
        this.onBlur = this.onBlur.bind(this);
        this.input = React.createRef();
        this.onChange = this.onChange.bind(this);
        this.fireChange = Debouncer(this.fireChange.bind(this), 500);
    }

    onFocus(event: React.FocusEvent<HTMLTextAreaElement>): void {
        const input = this.input ? this.input.current : null;
        if (!input) return;
        if (!input.classList) return;
        input.classList.remove("thm-bg3");
        input.classList.add("thm-bgSL");
    }

    onBlur(event: React.FocusEvent<HTMLTextAreaElement>): void {
        const input = this.input ? this.input.current : null;
        if (!input) return;
        if (!input.classList) return;
        input.classList.add("thm-bg3");
        input.classList.remove("thm-bgSL");
    }

    onChange(event: React.ChangeEvent<HTMLTextAreaElement>): void {
        const
            p = this.props,
            delay = castInteger(p.delay, 500);

        event.preventDefault();
        this.fireChange.delay = delay;
        this.fireChange(event.target.value);
    }

    fireChange(value: string) {
        const slot = this.props.onChange;
        if( typeof slot !== 'function' ) return;
        try {
            this.lastEmittedValue = value;
            slot( value );
        } catch(ex) {
            console.error("Exception thrown while calling onChange event handler: ", ex);
            console.error("Value was: ", value);
        }
    }

    render() {
        const
            p = this.props,
            value = castString(p.value, ""),
            wide = castBoolean(p.wide, false),
            width = castUnit(p.width, "auto"),
            cols = castInteger(p.cols, 40),
            rows = castInteger(p.rows, 6),
            id = nextId();
        const classes = ["tfw-view-TextArea"];
        if (wide) classes.push("wide");

        return (<div
                    className={classes.join(" ")}
                    style={{ width, maxWidth: width }}>
            <Label label={p.label}/>
            <textarea
                ref={this.input}
                className="thm-bg3 thm-ele-button"
                id={id}
                cols={cols}
                rows={rows}
                onChange={this.onChange}
                onFocus={this.onFocus}
                onBlur={this.onBlur}
                value={value !== this.lastEmittedValue ? value : undefined}/>
        </div>);
    }
}


let globalId = 0;
function nextId() {
    return `tfw-view-input-${globalId++}`;
}
