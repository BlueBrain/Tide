import * as React from "react"
import castString from "../converter/string"
import castBoolean from "../converter/boolean"
import castStringArray from "../converter/string-array"
import "./flex.css"

interface IFlexProp {
    wide?: boolean;
    classes?: string[] | string;
    dir?: "row" | "row-reverse" | "column" | "column-reverse" | "wide" | "narrow";
    wrap?: "nowrap" | "wrap" | "wrap-reverse";
    justifyContent?: "flex-start" | "flex-end" | "center" | "space-around" | "space-between" | "space-evenly";
    alignItems?: "stretch" | "flex-start" | "flex-end" | "center";
    children: React.ReactElement<any>[] | React.ReactElement<any>;
}

export default class Flex extends React.Component<IFlexProp, {}> {
    render() {
        const p = this.props;
        const wide = castBoolean(p.wide, true);
        const dir = castString(p.dir, "row");
        const style: React.CSSProperties = {
            justifyContent: castString(p.justifyContent, "space-around"),
            alignItems: castString(p.alignItems, "center"),
            flexWrap: castString(p.wrap, "wrap")
        };
        const classes = ["tfw-view-flex", `dir-${dir}`];
        classes.push(...castStringArray(p.classes));

        if (wide) classes.push("wide");

        return (
            <div className={classes.join(" ")} style={style}>{
                p.children
            }</div>
        )
    }
}
