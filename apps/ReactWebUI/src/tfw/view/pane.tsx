import * as React from "react";
import "./pane.css";
import "../theme.css";
import castString from "../converter/string";

interface IPaneSlot {
    label?: string;
}

export default class Pane extends React.Component<IPaneSlot, {}> {
    render() {
        const label = castString(this.props.label, "");

        return (
            <div className="tfw-view-pane thm-bg1">
                {label.length > 0 ? <header>{label}</header> : null}
                <div>{this.props.children}</div>
            </div>
        );
    }
}
