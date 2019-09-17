import React from "react"

import Gesture from '../../gesture'
import Label from '../label'
import castInteger from "../../converter/integer"
import castString from "../../converter/string"
import castFloat from "../../converter/double"

import "./slider.css"

interface ISliderProps {
    value: number,
    label?: string,
    text?: string,
    step?: number,
    min?: number,
    max?: number,
    onChange: (value: number) => void
}

export default class Slider extends React.Component<ISliderProps, {}> {
    private readonly ref: React.RefObject<HTMLDivElement> = React.createRef();

    handleTapOrMove = (evt: any) => {
        const div = this.ref.current;
        if (!div) return;
        const rect = div.getBoundingClientRect();
        const percent = clamp(evt.x / rect.width);
        const p = this.props;
        const a = castInteger(p.min, 0);
        const b = castInteger(p.max, 100);
        const min = Math.min(a, b);
        const max = Math.max(a, b);
        const step = Math.max(1e-9, castFloat(p.step, (max - min) / 1000));
        const value = this.constrainValue(
            min + (max - min) * percent, min, max, step
        );
        p.onChange(value);
    }

    componentDidMount() {
        const div = this.ref.current;
        if (!div) return;
        Gesture(div).on({
            tap: this.handleTapOrMove,
            pan: this.handleTapOrMove
        })
    }

    render() {
        const p = this.props;
        const a = castInteger(p.min, 0);
        const b = castInteger(p.max, 100);
        const min = Math.min(a, b);
        const max = Math.max(a, b);
        const step = Math.max(1e-9, castFloat(p.step, (max - min) / 1000));
        const label = castString(p.label, '');
        const value = this.constrainValue(p.value, min, max, step);
        const text = castString(p.text, `${value}`);

        return (<div className="tfw-view-Slider"
                     ref={this.ref}>
            <Label label={label}/>
            <div className="thm-ele-button thm-bgPD">
                <div className="thm-ele-nav thm-bgPL thm-fg"
                     style={{
                         marginLeft: `calc(${100 * (value - min) / (max - min)}% - 16px)`
                     }}>{text}</div>
            </div>
        </div>)
    }

    private constrainValue(value: number, min: number, max: number, step: number) {
        let v = Math.max(min, Math.min(value, max));
        if (step > 0) {
            return min + step * Math.floor(step / 2 + (v - min) / step);
        }
        return v;
    }
}


function clamp(value: number, min: number = 0, max: number = 1) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}
