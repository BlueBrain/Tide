import * as React from "react";
import "./color-button-list.css";
import "../theme.css";
import ColorButton from "./color-button";
import castString from "../converter/string";
import castInteger from "../converter/integer";
import castStringArray from "../converter/string-array";

interface IPaneSlot {
    label?: string;
    value?: string;
    colors?: string[];
    onChange?: (color: string) => void;
}

const DEFAULT_COLORS = ["#000000", "#FFFFFF"];

export default class Pane extends React.Component<IPaneSlot, {}> {
    constructor(props: IPaneSlot) {
        super(props);
        this.onChange = this.onChange.bind(this);
    }

    onChange(color: string): void {
        const slot = this.props.onChange;
        if (typeof slot === 'function') {
            slot(color);
        }
    }

    render() {
        const label = castString(this.props.label, "");
        const value = castString(this.props.value, "#000000").toLowerCase();
        const colors = castStringArray(this.props.colors, DEFAULT_COLORS);
        const elem = (
            <div className="tfw-view-color-button-list" >
                {label.length > 0 ? <header className="thm-bgPD">{label}</header> : null}
                <div> {
                    colors.map(color => {
                        const selected = (color.toLowerCase() == value);
                        return (<div key={color}
                            className={selected ? "selected" : ""} >
                            <ColorButton
                                value={color}
                                selected={selected}
                                onClick={this.onChange} />
                        </div>);
                    })
                } </div>
            </div>
        );
        return elem;
    }
}
