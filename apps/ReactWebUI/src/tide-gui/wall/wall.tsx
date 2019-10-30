import React from "react"
import Icon from '../../tfw/view/icon'
import Touchable from '../../tfw/view/touchable'
import { IWallInfo } from '../service/query'

import "./wall.css"

interface IWallProps {
    wall?: number,
    walls: IWallInfo[],
    onClick: (wallNumber: number) => void
}

export default class Wall extends React.Component<IWallProps, {}> {
    renderWall = (wall: IWallInfo) => {
        const enabled = wall.id !== this.props.wall
        const classes = ["wall-button"]
        if (enabled) {
            classes.push("thm-ele-button thm-bg2")
        } else {
            classes.push("disabled")
        }
        return <Touchable
                    key={wall.name}
                    classNames={classes}
                    enabled={enabled}
                    onClick={() => this.props.onClick(wall.id)}>
            <div className="power-status">
                <div className="icon">
                    <div className={wall.power ? 'power-on' : 'power-off'}>
                        <Icon content="tv" size={28} pen0="#ff0"/>
                    </div>
                    <div className={wall.power ? 'power-on' : 'power-off'}>
                        <Icon content="tv" size={28} pen0="#ff0"/>
                    </div>
                    <div>
                        <Icon content="tv" size={28}/>
                    </div>
                </div>
                <div className={wall.power ? 'on' : 'off'}>{wall.power ? 'ON' : 'OFF'}</div>
            </div>
            <div>
                <div>{wall.name}</div>
            </div>
            <div className="icon">
                <div>
                    <Icon content={wall.locked ? "locked" : "unlocked"}
                          size={28}/>
                </div>
            </div>
        </Touchable>
    }

    render() {
        return (
            <div className="tideGui-Wall">
                <h1>Please select a display wall</h1>
                <div>{
                    this.props.walls.length === 0 ?
                    <div className="wait">
                        <Icon content="wait" animate={true}/>
                        <div>Checking wall screens status...</div>
                    </div>
                    :
                    this.props.walls.map(this.renderWall)
                }</div>
            </div>
        )
    }
}
