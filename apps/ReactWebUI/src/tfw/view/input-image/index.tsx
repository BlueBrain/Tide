import React from "react"
import InputFile from "../input-file"
import Flex from "../../layout/flex"
import Combo from "../combo"
import Label from "../label"
import Intl from "../../intl"
import castString from "../../converter/string"
import castInteger from "../../converter/integer"
import castDouble from "../../converter/double"

import "./input-image.css"

const _ = Intl.make(require("./input-image.yaml"))

interface IInputImageProps {
    label?: string;
    value?: string;
    width: number;
    height: number;
    scale?: number;
    onChange?: (url: string) => void;
}

interface IInputImageState {
    size: "cover" | "contain" | "fill"
}

export default class InputImage extends React.Component<IInputImageProps, IInputImageState> {
    private readonly refCanvas: React.RefObject<HTMLCanvasElement> = React.createRef();
    private lastUrl: string = "";

    constructor( props: IInputImageProps ) {
        super( props );
        this.state = { size: "cover" };
        this.handleUpload = this.handleUpload.bind( this );
    }

    handleUpload(files: FileList) {
        const canvas = this.refCanvas.current;
        if( !canvas ) return;
        canvas.classList.remove("show");

        const that = this;
        const file = files[0];
        const reader = new FileReader();

        reader.addEventListener("load", function() {
            const url = reader.result;
            if( !url ) return;
            that.refresh(url).then(() => that.handleChange(canvas));
        });
        reader.readAsDataURL( file );
    }

    handleChange(canvas: HTMLCanvasElement) {
        const handle = this.props.onChange;
        if( typeof handle !== 'function' ) return;
        handle(canvas.toDataURL("image/png", 1));
    }

    private refresh(url: string) {
        if( url === this.lastUrl ) return;
        const canvas = this.refCanvas.current;
        if( !canvas ) return;
        const that = this;

        return new Promise((resolve, reject) => {
            const img = new Image();
            img.crossOrigin = "Anonymous";
            img.onload = () => {
                const ctx = canvas.getContext("2d");
                if( !ctx ) return;
                ctx.clearRect(0, 0, canvas.width, canvas.height);
                const { x, y, w, h } = computeSize(that.state.size, img, canvas);
                ctx.drawImage( img, x, y, w, h );
                canvas.classList.add("show");
                that.lastUrl = url;
                resolve();
            };
            img.onerror = () => {
                canvas.classList.remove("show");
                reject();
            };
            img.src = url;
        });
    }

    componentDidMount() {
        this.refresh(castString(this.props.value, ""));
    }

    componentDidUpdate() {
        this.refresh(castString(this.props.value, ""));
    }

    render() {
        const p = this.props;
        const label = castString(p.label, "").trim();
        const width = castInteger(p.width);
        const height = castInteger(p.height);
        const scale = castDouble(p.scale);
        const viewW = width * scale;
        const viewH = height * scale;

        return (<div
                    style={{
                        width: `${viewW}px`
                    }}>
            <Label label={label}/>
            <div
                className="image tfw-view-InputImage thm-ele-button thm-bg2"
                style={{
                    width: `${viewW}px`,
                    height: `${viewH}px`
                }}
                title={this.props.value}>
                <canvas
                    ref={this.refCanvas}
                    style={{
                        width: `${viewW}px`,
                        height: `${viewH}px`
                    }}
                    width={width}
                    height={height}></canvas>
            </div>
            <Flex justifyContent="space-between" alignItems="center">
                <InputFile
                    onClick={this.handleUpload}
                    accept="image/*"
                    label={_("upload")}
                    small={true}
                    icon="import"
                    wide={true}
                    flat={true}/>
                <Combo value={this.state.size}
                    onChange={size => this.setState({ size })}>{
                    ["fill", "cover", "contain"].map( label => (
                        <div key={label} className="tfw-view-InputImage-item">
                            <div className={`icon ${label}`}></div>
                        </div>
                    ))
                }</Combo>
            </Flex>
        </div>)
    }
}


function computeSize(size: "fill" | "contain" | "cover",
        img: HTMLImageElement,
        canvas: HTMLCanvasElement) {
    if( size === "fill" ) return { x: 0, y: 0, w: canvas.width, h: canvas.height };
    if( size === "contain" ) return computeSizeContain( img, canvas );
    return computeSizeCover( img, canvas );
}

function computeSizeContain(img: HTMLImageElement, canvas: HTMLCanvasElement) {
    const scaleW = canvas.width / img.width;
    const scaleH = canvas.height / img.height;
    const scale = Math.min(scaleW, scaleH);
    const w = img.width * scale;
    const h = img.height * scale;
    const x = (canvas.width - w) / 2;
    const y = (canvas.height - h) / 2;
    return { x, y, w, h };
}


function computeSizeCover(img: HTMLImageElement, canvas: HTMLCanvasElement) {
    const scaleW = canvas.width / img.width;
    const scaleH = canvas.height / img.height;
    const scale = Math.max(scaleW, scaleH);
    const w = img.width * scale;
    const h = img.height * scale;
    const x = (canvas.width - w) / 2;
    const y = (canvas.height - h) / 2;
    return { x, y, w, h };
}
