import * as React from "react"
import "./fieldset.css"

interface IFieldsetProps {
    label: string;
    children: React.ReactElement<any>[] | React.ReactElement<any>;
}

export default class Fieldset extends React.Component<IFieldsetProps, {}> {
    render() {
        return (
            <fieldset className="thm-bg2">
                <legend className="thm-bgPD">{this.props.label}</legend>
                {this.props.children}
            </fieldset>
        )
    }
}
