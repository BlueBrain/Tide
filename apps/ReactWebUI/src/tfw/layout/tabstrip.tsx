import React from "react"
import castArray from "../converter/array"
import castInteger from "../converter/integer"
import castStringArray from "../converter/string-array"

import "./tabstrip.css"

interface ITabstripProps {
    value: number;
    headers: string[];
    classes?: string | string[];
    children: React.ReactElement<any>[];
    onChange?: (value: number) => void;
}

export default class Tabstrip extends React.Component<ITabstripProps, {}> {
    constructor(props: ITabstripProps) {
        super(props);
        this.handleChange = this.handleChange.bind(this);
    }

    handleChange(value: number) {
        const handler = this.props.onChange;
        if (typeof handler !== 'function') return;
        handler(value);
    }

    render() {
        const children = castArray(this.props.children);
        const value = castInteger(this.props.value, 0);
        const headers = castStringArray(this.props.headers);
        const child = children[value];
        const handler = this.handleChange;
        const classes = ["tfw-layout-tabstrip"];
        classes.push(...castStringArray(this.props.classes));

        return (
            <div className={classes.join(" ")}>
                <header>
                    {
                        headers.map((header: string, index: number) => (
                            <button
                                key={index}
                                onClick={() => handler(index)}
                                className={value === index ? "selected thm-bg2" : ""}>{
                                    header
                                }</button>
                        ))
                    }
                    <div className="void" />
                </header>
                <div className="thm-bg2">{child}</div>
            </div>
        );
    }
}
