import React from "react"
import Icon from "../icon"
import Label from "../label"
import Color from "../../color"

import "./input-color.css"

interface IInputColorProps {
    value: string;
    label?: string;
    wide?: boolean;
    onChange?: (value: string) => void;
}

interface IInputColorState {
    value: string
}

const RX_COLOR = /^#[0-9a-f]{6}$/gi;

export default class InputColor extends React.Component<IInputColorProps, IInputColorState> {
    private readonly refInput: React.RefObject<HTMLInputElement> = React.createRef();

    constructor( props: IInputColorProps ) {
        super( props );
        this.state = { value: "" };
        this.handleChange = this.handleChange.bind( this );
    }

    handleChange(event: React.ChangeEvent<HTMLInputElement>) {
        const input = this.refInput.current;
        if( !input ) return;
        const handler = this.props.onChange;
        if (typeof handler !== 'function') return;
        try {
            const value = input.value.trim();
            if( !RX_COLOR.test(value) ) {
                input.classList.add("invalid");
                return;
            }
            input.classList.remove("invalid");
            handler( value );
        } catch(ex) {
            console.error("Error in handleChange(): ", );
            console.error(ex);
        }
    }
    componentDidMount() {
        this.refresh( this.props.value );
    }

    componentDidUpdate() {
        if( this.state.value !== this.props.value ) {
            this.refresh( this.props.value );
        }
    }

    refresh(value: string) {
        const input = this.refInput.current;
        if( !input ) return;
        input.value = Color.normalize(value);
        this.setState({ value: value });
    }

    render() {
        const p = this.props;
        const label = p.label;
        const color = new Color(this.state.value);
        const value = color.stringify();
        const fg = color.luminanceStep() ? "#000" : "#FFF";

        return (<div className="tfw-view-InputColor">
            <div>
                { label ? <Label label={label}/> : null }
                <div className="thm-ele-button thm-bg3">
                    <input
                        ref={this.refInput}
                        maxLength={7}
                        onChange={this.handleChange}
                        defaultValue={value}
                        style={{
                            color: fg,
                            background: value
                        }}/>
                    <div className="thm-bgP" style={{ background: value }}>
                        <Icon content="edit" pen0={fg} size="1.5rem"/>
                    </div>
                </div>
            </div>
        </div>)
    }
}
