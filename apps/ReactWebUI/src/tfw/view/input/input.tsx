import * as React from "react"
import "./input.css"
import castInteger from "../../converter/integer"
import castBoolean from "../../converter/boolean"
import castString from "../../converter/string"
import castUnit from "../../converter/unit"
import Label from "../label"

interface IStringSlot {
    (value: string): void;
}

interface IInputProps {
    value: string;
    label?: string;
    wide?: boolean;
    size?: number;
    focus?: boolean;
    width?: string;
    enabled?: boolean;
    placeholder?: string;
    type?: "text" | "password" | "submit" | "color" | "date"
    | "datetime-local" | "email" | "month" | "number" | "range"
    | "search" | "tel" | "time" | "url" | "week";
    validator?: (value: string) => boolean | RegExp;
    onValidation?: (validation: boolean) => void;
    onChange?: IStringSlot,
    onEnterPressed?: (value: string) => void
}

export default class Input extends React.Component<IInputProps, {}> {
    private readonly input: React.RefObject<HTMLInputElement> = React.createRef();

    handleKeyDown = (evt: React.KeyboardEvent<HTMLInputElement>) => {
        if (evt.key === "Enter") {
            evt.preventDefault();
            evt.stopPropagation();
            const { onEnterPressed, value } = this.props;
            if (typeof onEnterPressed === 'function') {
                onEnterPressed(value);
            }
        }
    }

    onFocus = (event: React.FocusEvent<HTMLInputElement>): void => {
        const input = this.input ? this.input.current : null;
        if (!input) return;
        if (!input.classList) return;
        input.classList.remove("thm-bg3");
        input.classList.add("thm-bgSL");
        if (this.props.type !== 'number') {
            // setSelectionRange fails for "number" input.
            input.setSelectionRange(0, input.value.length);
        }
    }

    onBlur = (event: React.FocusEvent<HTMLInputElement>): void => {
        const input = this.input ? this.input.current : null;
        if (!input) return;
        if (!input.classList) return;
        input.classList.add("thm-bg3");
        input.classList.remove("thm-bgSL");
    }

    onChange = (event: React.ChangeEvent<HTMLInputElement>): void => {
        //if (!this.checkValidity(event.target.value)) return;
        const { onChange } = this.props;
        if (typeof onChange === 'function') {
            event.preventDefault();
            onChange(event.target.value);
        }
    }

    render() {
        const
            p = this.props,
            type = castString(p.type, "text"),
            label = castString(p.label, ""),
            value = castString(p.value, ""),
            placeholder = castString(p.placeholder, ""),
            wide = castBoolean(p.wide, false),
            focus = castBoolean(p.focus, false),
            enabled = castBoolean(p.enabled, true),
            size = Math.max(1, castInteger(p.size, 8)),
            width = castUnit(p.width, "auto"),
            id = nextId();
        const classes = ["tfw-view-input"];
        if (wide) classes.push("wide");
        /*
        const header = (error ? <div className="error">{error}</div> :
            (label ? (<Label htmlFor={id} label={label}/>) : null));
        */
        const inputClassName = "thm-ele-button "
            + (enabled ? 'thm-bg3' : 'thm-bg0');
        return (<div
                    className={classes.join(" ")}
                    style={{ width, maxWidth: width }}>
            <Label htmlFor={id} label={label}/>
            <input
                ref={this.input}
                autoFocus={focus}
                className={inputClassName}
                placeholder={placeholder}
                disabled={!enabled}
                type={type}
                id={id}
                size={size}
                onFocus={this.onFocus}
                onBlur={this.onBlur}
                onChange={this.onChange}
                onKeyDown={this.handleKeyDown}
                value={value}/>
        </div>);
    }
}


let globalId = 0;
function nextId() {
    return `tfw-view-input-${globalId++}`;
}
