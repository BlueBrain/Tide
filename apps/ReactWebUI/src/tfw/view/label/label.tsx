import React from "react"

import "./label.css"

interface ILabelProps {
    label?: string;
    htmlFor?: string;
}

export default class Label extends React.Component<ILabelProps, {}> {
    render() {
        const label = this.props.label;
        if( typeof label !== 'string' || label.trim().length === 0 ) return null;
        return (<label className="tfw-view-Label" htmlFor={this.props.htmlFor}>{
            this.props.label
        }</label>)
    }
}
