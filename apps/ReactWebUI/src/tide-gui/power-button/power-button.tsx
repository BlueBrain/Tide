import React from "react"
import Icon from "../../tfw/view/icon"
import "./power-button.css"

interface TPowerButtonProps {
    status: string  // "on" | "off" | "?"
}

export default class PowerButton extends React.Component<TPowerButtonProps> {
    private isOn() {
        return this.props.status.trim().toUpperCase() === 'ON'
    }

    private get label() {
        switch (this.props.status.trim().toUpperCase()) {
            case 'ON': return 'ON'
            case 'OFF': return 'OFF'
            default: return '?'
        }
    }

    render() {
        const classes = ['tideGui-PowerButton']

        return (<div className={classes.join(' ')}>
            <div className="icon">
                {/* Tow DIVs are used for glowing effect. */}
                <div className={this.isOn() ? 'power-on' : 'power-off'}>
                    <Icon content="tv" size={28} pen0="#00f"/>
                </div>
                <div className={this.isOn() ? 'power-on' : 'power-off'}>
                    <Icon content="tv" size={28} pen0="#0ff"/>
                </div>
                <div>
                    <Icon content="tv" size={28}/>
                </div>
            </div>
            <div className="label">{this.label}</div>
        </div>)
    }
}
