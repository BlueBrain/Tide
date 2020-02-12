import React from "react"
import castUnit from "../converter/unit"

interface ISpaceProps {
    size: string;
}

export default class Space extends React.Component<ISpaceProps, {}> {
    render() {
        const style = {
            content: ".",
            color: "transparent",
            display: "block",
            height: castUnit(this.props.size, "0"),
            lineHeight: castUnit(this.props.size, "0")
        };
        return <div style={style}></div>
    }
}
