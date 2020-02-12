import React from "react"
import castArray from "../converter/array"
import castString from "../converter/string"
import castBoolean from "../converter/boolean"
import Button from "../view/button"
import Gesture from "../gesture"
import "./sidemenu.css"

interface ISidemenuProps {
    show?: boolean;
    head?: string;
    menu?: React.ReactElement<HTMLDivElement>;
    body?: React.ReactElement<HTMLDivElement>;
    classes?: string[] | string;
    onShowChange?: (isMenuVisible: boolean) => void;
}

export default class Sidemenu extends React.Component<ISidemenuProps, {}> {
    refMenu = React.createRef<HTMLDivElement>();

    constructor(props: ISidemenuProps) {
        super(props);
        this.handleShowChange = this.handleShowChange.bind(this);
    }

    componentDidMount() {
        const menu = this.refMenu.current;
        if (!menu) return;
        Gesture(menu).on({
            swipeleft: () => {
                const handler = this.props.onShowChange;
                if (typeof handler === 'function') handler(false);
            }
        });
    }
    handleShowChange() {
        const handler = this.props.onShowChange;
        if (typeof handler !== 'function') return;
        handler(!castBoolean(this.props.show, window.innerWidth > 480));
    }

    render() {
        const show = castBoolean(this.props.show, window.innerWidth > 480);
        const head = castString(this.props.head, "");
        const classes = ["tfw-layout-sidemenu thm-bg0"].concat(castArray(this.props.classes));
        if (show) classes.push("show");

        return (
            <div className={classes.join(" ")} >
                <div className="body thm-bg0" >{this.props.body}</div>
                <div className="menu thm-ele-nav thm-bg1" ref={this.refMenu}>
                    <header className="thm-ele-nav thm-bgPD" >{head}</header>
                    <menu> {this.props.menu} </menu>
                </div>
                <div className="icon thm-bgPD" >
                    <Button icon="menu" flat={true} onClick={this.handleShowChange} />
                </div>
            </div>
        );
    }
}
