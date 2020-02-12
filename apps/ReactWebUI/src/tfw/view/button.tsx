import * as React from "react"
import Icon from "./icon"
import "./button.css"
import Touchable from "../behavior/touchable"
import castArray from "../converter/array"
import castString from "../converter/string"
import castBoolean from "../converter/boolean"

interface IButtonProps {
    label?: string;
    icon?: string;
    wide?: boolean;
    wait?: boolean;
    flat?: boolean;
    small?: boolean;
    warning?: boolean;
    enabled?: boolean;
    classes?: string[] | string;
    tag?: any;
    onClick?: (tag: any) => void;
}

export default class Button extends React.Component<IButtonProps, {}> {
    readonly touchable: Touchable;
    readonly ref: React.RefObject<HTMLButtonElement>;

    constructor(props: IButtonProps) {
        super(props);
        this.handleClick = this.handleClick.bind(this);
        this.touchable = new Touchable({ onTap: this.handleClick });
        this.ref = React.createRef();
    }

    componentDidMount() {
        const element = this.ref.current;
        if (!element) return;
        this.touchable.element = element;
    }

    handleClick() {
        const slot = this.props.onClick;
        if (typeof slot !== 'function') return;
        slot(this.props.tag);
    }

    render() {
        const p = this.props,
            label = castString(p.label, ""),
            icon = castString(p.icon, ""),
            wide = castBoolean(p.wide, false),
            wait = castBoolean(p.wait, false),
            flat = castBoolean(p.flat, false),
            small = castBoolean(p.small, false),
            enabled = !wait && castBoolean(p.enabled, true),
            warning = castBoolean(p.warning, false),
            classes = ["tfw-view-button"].concat(castArray(p.classes));
        if (wide) classes.push("wide");
        if (flat) {
            classes.push("flat");
            classes.push(warning ? "thm-fgS" : "thm-fgP");
        }
        else {
            if (enabled) classes.push("thm-ele-button");
            classes.push(warning ? "thm-bgS" : "thm-bgP");
        }
        if (small) classes.push("small");
        if (label.length === 0) classes.push("floating");

        this.touchable.enabled = enabled;
        return (
            <button ref={this.ref}
                className={classes.join(" ")}
                disabled={!enabled}>
                {icon.length > 0
                    ? <Icon content={icon}
                        animate={wait}
                        size={`${small ? 1.375 : 1.875}rem`} />
                    : null}
                {label.length > 0
                    ? <div className="text" > {label} </div>
                    : null}
            </button>);
    }
}
