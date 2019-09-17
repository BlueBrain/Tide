import * as React from "react";
import castBoolean from "../converter/boolean";
import castInteger from "../converter/integer";
import castUnit from "../converter/unit";
import { iconsBook, TIconDefinition } from "../icons";
import "./icon.css";

enum EnumPenColor { B, W, P, PD, PL, S, SD, SL }

interface IIconProps {
    visible?: boolean;
    content?: string | TIconDefinition;
    size?: string | number;
    enabled?: boolean;
    animate?: boolean;
    flipH?: boolean;
    flipV?: boolean;
    rotate?: number;
    pen0?: EnumPenColor | string;
    pen1?: EnumPenColor | string;
    pen2?: EnumPenColor | string;
    pen3?: EnumPenColor | string;
    pen4?: EnumPenColor | string;
    pen5?: EnumPenColor | string;
    pen6?: EnumPenColor | string;
    pen7?: EnumPenColor | string;
    onClick?: () => void;
    onHide?: () => void;
}

export default class Icon extends React.Component<IIconProps, {}> {
    static isValidIconName(name: string): boolean {
        return typeof iconsBook[name] !== 'undefined';
    }

    static getAllIconNames(): string[] {
        return Object.keys(iconsBook).sort();
    }

    private refIcon: any;
    private visible: boolean;
    private timeoutHandle: number;

    constructor(props: IIconProps) {
        super(props);
        this.refIcon = React.createRef();
        this.visible = false;
        this.timeoutHandle = 0;
    }

    triggerVisibleAnimation() {
        const
            elemIcon = this.refIcon.current,
            visible = this.visible;
        if( !elemIcon ) return;
        if (this.timeoutHandle) {
            window.clearTimeout(this.timeoutHandle);
            this.timeoutHandle = 0;
        }
        requestAnimationFrame(() => {
            if (visible) elemIcon.classList.remove("zero");
            else {
                elemIcon.classList.add("zero");
                const slot = this.props.onHide;
                if (typeof slot === 'function') {
                    this.timeoutHandle = window.setTimeout(slot, 300);
                }
            }
        });
    }

    render() {
        const
            p = this.props,
            visible = castBoolean(p.visible, true),
            enabled = castBoolean(p.enabled, true),
            animate = castBoolean(p.animate, false),
            flipH = castBoolean(p.flipH, false),
            flipV = castBoolean(p.flipV, false),
            size = castUnit(p.size, "28px"),
            content = castContent(p.content),
            rotate = castInteger(p.rotate, 0),
            onClick = p.onClick,
            classes = ["tfw-view-icon"];
        const svgContent = createSvgContent(content, p);
        if (!svgContent) return null;

        if (!enabled) classes.push("disabled");
        if (animate) classes.push("animate");
        if (visible) classes.push("zero");
        if (typeof onClick === 'function' && enabled) classes.push("active");

        let transform = "";
        if (rotate !== 0) {
            transform += `rotate(${rotate}deg) `;
        }
        if (flipH || flipV) {
            transform += `scale(${flipH ? -1 : 1},${flipV ? -1 : 1})`;
        }
        const style: React.CSSProperties = { width: size, height: size };
        if (transform.length > 0) style.transform = transform;

        requestAnimationFrame(() => this.triggerVisibleAnimation());
        this.visible = visible;

        return (
            <svg className={classes.join(" ")}
                ref={this.refIcon}
                viewBox="-65 -65 130 130"
                preserveAspectRatio="xMidYMid"
                width={size}
                height={size}
                onClick={enabled ? onClick : undefined}
                style={style}>
                {svgContent}
                < g strokeWidth="6" fill="none" strokeLinecap="round" strokeLinejoin="round" >
                    {createSvgContent(content, p)}
                </g>
            </svg>
        );
    }
}

function createSvgContent(def: TIconDefinition, props: IIconProps, key: string = ""): any {
    const
        elementName = def[0],
        { attributes, children } = parseDef(def);

    if (typeof elementName === 'undefined') return <g key={key}></g>;

    const attribs: React.Attributes = manageColors({ ...attributes }, props);
    if (key.length > 0) attribs.key = key;

    return React.createElement(
        elementName,
        attribs,
        children.map((child, index) => createSvgContent(child, props, `${index}`))
    ) as React.ReactSVGElement;
}

const CLASSES = ["P", "PL", "PD", "S", "SL", "SD"];
const PENS = "01234567";

/**
 * If the value of the attribute "fill" is an element of CLASSES,
 * it will be removed and replace with a class.
 * Same thing for the attibute "stroke".
 *
 * @example
 * manageColors({ fill: "1" }) === { className: "thn-svg-fill-1" }
 *
 * @param   attribs [description]
 * @param   props   [description]
 * @returns         [description]
 */
function manageColors(attribs: { [key: string]: any }, props: IIconProps) {
    // @TODO For special forms of "fill" and "stroke", add classes.
    const classes = (attribs.className || "").split(" ");

    ["fill", "stroke"].forEach(attribName => {
        const attribValue = `${attribs[attribName]}`;
        if (typeof attribValue === 'undefined') return;
        if (attribValue === 'undefined') return;
        const key = `${attribValue}`.toUpperCase();
        if (CLASSES.indexOf(key) === -1 && PENS.indexOf(key) === -1) return;
        if (PENS.indexOf(attribValue) !== -1 ) {
            // If we find fill="3", then we must look for property pen3.
            const pen = props[`pen${attribValue}`];
            if( typeof pen === 'string' ) {
                if( CLASSES.indexOf(pen) === -1 ) {
                    attribs[attribName] = pen;
                } else {
                    delete attribs[attribName];
                    classes.push(`thm-svg-${attribName}${pen}`);
                }
                return;
            }
        }
        delete attribs[attribName];
        classes.push(`thm-svg-${attribName}${key}`);
    });

    attribs.className = classes.join(" ").trim();
    return attribs;
}

function parseDef([name, arg1, arg2]: TIconDefinition) {
    let attributes: undefined | {},
        children: undefined | any[];

    if (Array.isArray(arg1)) children = arg1;
    else if (typeof arg1 !== 'undefined') attributes = arg1;
    if (Array.isArray(arg2)) children = arg2;
    else if (typeof arg2 !== 'undefined') attributes = arg2;

    if (typeof attributes === 'undefined') attributes = {};
    if (typeof children === 'undefined') children = [];

    return { attributes, children };
}

function castContent(content: string | TIconDefinition = "ok"): TIconDefinition {
    if (Array.isArray(content)) return content;
    return iconsBook[content] || [];
}
