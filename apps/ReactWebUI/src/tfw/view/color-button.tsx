import * as React from "react";
import "./color-button.css";
import Color from "../color";

interface IStringSlot {
    (value: string): void;
}

interface IColorButtonProps {
    value?: string;
    size?: TLength;
    selected?: boolean;
    onClick?: IStringSlot;
}

export default class ColorButton extends React.Component<IColorButtonProps, {}> {
    readonly color: Color;

    constructor(props: IColorButtonProps) {
        super(props);
        this.onClick = this.onClick.bind(this);
        this.color = new Color();
    }

    onClick() {
        const slot = this.props.onClick;
        if (typeof slot === 'function') {
            slot(this.color.stringify());
        }
    }

    render() {
        const color = this.color;
        color.parse(this.props.value);

        const
            cls = ["tfw-view-color-button"],
            luminance = color.luminanceStep(),
            size = this.props.size ? this.props.size : "2rem";

        if (this.props.selected) {
            cls.push("thm-ele-button-raised");
        } else {
            cls.push("thm-ele-button");
        }
        cls.push(`lum${luminance}`);

        return (<button
            className={cls.join(" ")}
            onClick={this.onClick}
            style={{
                backgroundColor: color.stringify(),
                width: size,
                height: size
            }}>.</button>)
    }
}
