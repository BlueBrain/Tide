import * as React from "react"
import "./list.css"
import Debouncer from "../../debouncer";
import castUnit from "../../converter/unit"
import castBoolean from "../../converter/boolean"
import castInteger from "../../converter/integer"
import castString from "../../converter/string"
import Gesture from "../../gesture"

interface IListProps {
    // Array of all the items to hold.
    items: any[];
    className?: string;
    // Function which takes an item and return a component.
    mapper: (item: any) => React.ReactElement;
    itemHeight?: number;
    width?: string;
    height?: string;
    animateRefresh?: boolean;
    onRefreshAsked?: () => void;
}

interface IListState {
    items: any[];
}

export default class List extends React.Component<IListProps, IListState> {
    private readonly refMain: React.RefObject<HTMLDivElement>;
    private readonly refHead: React.RefObject<HTMLDivElement>;
    private readonly refBody: React.RefObject<HTMLDivElement>;
    private readonly refTail: React.RefObject<HTMLDivElement>;
    private lastFirstItemIndex: number;
    private lastVisibleItemsCount: number;
    private lastItemsArray: any[];
    private itemHeight: number = 36;

    constructor(props: IListProps) {
        super(props);
        this.refMain = React.createRef();
        this.refHead = React.createRef();
        this.refBody = React.createRef();
        this.refTail = React.createRef();
        this.onScroll = Debouncer(this.onScroll.bind(this), 30);
        this.onPanDown = this.onPanDown.bind(this);
        this.lastFirstItemIndex = -1;
        this.lastVisibleItemsCount = -1;
        this.lastItemsArray = [];
        this.state = { items: [] };
    }

    onPanDown() {
        const main = this.refMain.current;
        if (!main) return;
        if (main.scrollTop > 0) return;

        const onRefresh = this.props.onRefreshAsked;
        if (typeof onRefresh !== 'function') return;
        onRefresh();
    }

    onScroll(evt: any = null): void {
        const main = this.refMain.current;
        const head = this.refHead.current;
        const body = this.refBody.current;
        const tail = this.refTail.current;
        if (!main || !head || !body || !tail) return;

        if (this.props.animateRefresh) main.scrollTop = 0;

        const rect = main.getBoundingClientRect();
        const top = Math.floor(main.scrollTop);
        const height = Math.floor(rect.height);
        const itemsCount = this.props.items.length;
        const firstItemIndex = Math.min(
            itemsCount, Math.floor(top / this.itemHeight));
        const visibleItemsCount = Math.min(
            itemsCount - firstItemIndex,
            1 + Math.ceil(height / this.itemHeight)
        );
        const tailCount = this.props.items.length - firstItemIndex - visibleItemsCount;

        head.style.height = `${this.itemHeight * firstItemIndex}px`;
        body.style.height = `${this.itemHeight * visibleItemsCount}px`;
        tail.style.height = `${this.itemHeight * tailCount}px`;

        main.scrollTop = top;

        if (this.lastFirstItemIndex !== firstItemIndex
            || this.lastVisibleItemsCount !== visibleItemsCount) {
            this.lastFirstItemIndex = firstItemIndex;
            this.lastVisibleItemsCount = visibleItemsCount;
            this.setState({
                items: this.props.items.slice(firstItemIndex, firstItemIndex + visibleItemsCount)
            });
        }

        // Resize background image for head and tail.
        const backgroundSize = `${main.clientWidth}px ${this.itemHeight}px`;
        head.style.backgroundSize = backgroundSize;
        tail.style.backgroundSize = backgroundSize;
    }

    componentDidMount() {
        window.addEventListener("resize", this.onScroll, false);
        const main = this.refMain.current;
        if (!main) return;
        Gesture(main).on({
            pandown: evt => {
                if (main.scrollTop > 0) return;
                evt.clear();
                main.classList.add("animate-refresh");
            },
            swipedown: evt => {
                if (main.scrollTop > 0) return;
                evt.clear();
                this.onPanDown();
            },
            up: () => main.classList.remove("animate-refresh")
        });
    }

    componentWillUnmount() {
        window.removeEventListener("resize", this.onScroll, false);
    }

    render() {
        this.itemHeight = castInteger(this.props.itemHeight, 36);
        if (this.lastItemsArray !== this.props.items) {
            this.lastVisibleItemsCount = -1;
            this.lastItemsArray = this.props.items;
        }
        this.onScroll();

        if (typeof this.props.mapper !== 'function') {
            console.error("props =", this.props);
            throw Error(`<List mapper="..." ...> Attribute mapper must be a function which returns a ReactElement!`);
        }
        let children = null;
        try {
            children = this.state.items
                .map(this.props.mapper)
                .map(content => (
                    <div className="item" key={content.key}
                        style={{
                            height: `${this.itemHeight}px`,
                            minHeight: `${this.itemHeight}px`,
                            maxHeight: `${this.itemHeight}px`
                        }}>{content}</div >
                ));
        }
        catch (ex) {
            throw Error(`<List mapper="..." ...> Attribute mapper returns an exception:\n${ex}`);
        }

        const width = castUnit(this.props.width, "auto");
        const height = castUnit(this.props.height, "auto");
        const animateRefresh = castBoolean(this.props.animateRefresh, false);
        const classes = ["tfw-view-list"];
        if (animateRefresh) classes.push("animate-refresh", "refreshing");
        const className = `${classes.join(' ')} ${castString(this.props.className, '')}`

        return (
            <div className={className}
                style={{ width, height }}
                onScroll={this.onScroll}
                ref={this.refMain}>
                <div className="space" ref={this.refHead} />
                <div className="body" ref={this.refBody}>{
                    children
                }</div>
                <div className="space" ref={this.refTail} />
            </div>
        );
    }
}
