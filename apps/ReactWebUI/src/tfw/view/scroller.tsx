import * as React from "react"
import castStringArray from "../converter/string-array"
import castBoolean from "../converter/boolean"
import castInteger from "../converter/integer"
import Gesture from "../gesture"
import Icon from "./icon"
import "./scroller.css"

type THandleChange = (arg: { top: number; max: number }) => void;

interface IScrollerProps {
    classes?: string[] | string;
    refreshable?: boolean;
    refreshing?: boolean;
    paddingTop?: number;
    onScrollChange?: THandleChange;
    onRefreshAsked?: () => void;
    children?: React.ReactElement<any> | React.ReactElement<any>[];
}

export default class Scroller extends React.Component<IScrollerProps, {}> {
    private readonly ref: React.RefObject<HTMLDivElement> = React.createRef();

    onPanDown() {
        const main = this.ref.current;
        if (!main || main.scrollTop > 0) return;

        const onRefresh = this.props.onRefreshAsked;
        if (typeof onRefresh !== 'function') return;
        onRefresh();
    }

    componentDidMount() {
        const div = this.ref.current;
        if (!div) return;
        Gesture(div).on({
            pandown: evt => {
                if (!castBoolean(this.props.refreshable, false)) return;
                if (div.scrollTop > 0) return;
                evt.clear();
                div.classList.add("animate-refresh");
            },
            swipedown: evt => {
                if (!castBoolean(this.props.refreshable, false)) return;
                if (div.scrollTop > 0) return;
                evt.clear();
                this.onPanDown();
            },
            up: () => div.classList.remove("animate-refresh")
        });
        div.addEventListener("scroll", () => {
            const handler = this.props.onScrollChange;
            if (typeof handler !== 'function') return;
            handler({ top: div.scrollTop, max: div.scrollHeight - div.clientHeight });
        }, false);
    }

    render() {
        const paddingTop = castInteger(this.props.paddingTop, 0);
        const classes = castStringArray(this.props.classes, []);
        classes.push("tfw-view-scroller");
        if (castBoolean(this.props.refreshing, false)) {
            classes.push("refreshing");
        }
        if (castBoolean(this.props.refreshable, false)) {
            return (<div className={classes.join(" ")} ref={this.ref}>
                <div className="body" style={{paddingTop: `${paddingTop}px`}}>{
                    this.props.children
                }</div>
                <div className="screen" style={{ top: `${paddingTop}px`}}>
                    <div className="thm-bgSD">
                        <Icon content="refresh" animate={true} />
                    </div>
                </div>
            </div>);
        }
        return (<div
                    style={{paddingTop: `${paddingTop}px`}}
                    className={classes.join(" ")}
                    ref={this.ref}>{
            this.props.children
        }</div>);
    }
}
