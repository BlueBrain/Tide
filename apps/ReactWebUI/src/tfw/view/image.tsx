import React from "react"
import castUnit from "../converter/unit"
import castStringArray from "../converter/string-array"
import "./image.css"

interface IImageProps {
    src: string;
    width?: string | number;
    height?: string | number;
    classes?: string | string[];
}

export default class Pack extends React.Component<IImageProps, {}> {
    private readonly ref: React.RefObject<HTMLDivElement> = React.createRef();
    private imageHasBeenLoaded = false;

    componentDidMount() {
        this.refresh();
    }

    componentDidUpdate() {
        this.refresh();
    }

    private refresh() {
        const div = this.ref.current;
        if (!div) return;

        if( this.imageHasBeenLoaded ) return;
        this.imageHasBeenLoaded = true;

        const img = new Image();
        div.classList.remove("show");
        img.onload = () => {
            div.classList.add("show");
        }
        img.onerror = () => {
            console.error("Unable to load image: ", this.props.src);
        }
        img.src = this.props.src;
    }

    render() {
        const p = this.props;
        const w = castUnit(p.width, "");
        const h = castUnit(p.width, "");
        const style: { width?: string, height?: string, backgroundImage: string } = {
            backgroundImage: `url(${p.src})`
        };
        if( w !== "" ) style.width = w;
        if( h !== "" ) style.height = w;
        const classes = castStringArray(p.classes, []);
        classes.push("tfw-view-image");
        if( this.imageHasBeenLoaded ) classes.push("show");

        return (<div ref={this.ref}
            style={style}
            className={classes.join(" ")} />);
    }
}
