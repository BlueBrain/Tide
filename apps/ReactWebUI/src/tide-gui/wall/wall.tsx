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

const DEFAULT_WALLS: IWallInfo[] = [
  {
    "name": "B1.00 Ground Floor",
    "id": 0,
    "width": 7716,
    "height": 3264,
    "locked": false,
    "power": false,
    "powerIsUndef": false
  },
  {
    "name": "B1.03 Third Floor (coffee area)",
    "id": 3,
    "width": 7720,
    "height": 2160,
    "locked": false,
    "power": false,
    "powerIsUndef": false
  },
  {
    "name": "B1.04 OpenDeck",
    "id": 4,
    "width": 11940,
    "height": 3424,
    "locked": false,
    "power": false,
    "powerIsUndef": false
  },
  {
    "name": "B1.05 Fifth Floor",
    "id": 5,
    "width": 5784,
    "height": 3264,
    "locked": false,
    "power": false,
    "powerIsUndef": false
  },
  {
    "name": "B1.06 Sixth Floor",
    "id": 6,
    "width": 7716,
    "height": 3264,
    "locked": false,
    "power": false,
    "powerIsUndef": false
  }
]


export default class Wall extends React.Component<IWallProps, {}> {
    renderWall = (wall: IWallInfo) => {
        const enabled = wall.id !== this.props.wall
        const classes = ["wall-button"]
        if (enabled) {
            classes.push("thm-ele-nav", "thm-bg3")
        } else {
            // "active" means that this is the currently active Wall.
            classes.push("active", "thm-bg1")
        }
        return <Touchable
                    key={wall.name}
                    classNames={classes}
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
                <div className={wall.power ? 'on' : 'off'}>{getPowerLabel(wall)}</div>
            </div>
            <div>
                <div>{wall.name}</div>
                {
                    wall.lastInteraction &&
                    <div className="date">{`Last interaction ${
                        getSmartDate(wall.lastInteraction)}`}</div>
                }
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
        const walls = this.props.walls.length === 0 ? DEFAULT_WALLS : this.props.walls
        return (
            <div className="tideGui-Wall">
                <h1>Please select a display wall</h1>
                <div>{
                    walls.map(this.renderWall)
                }</div>
            </div>
        )
    }
}


/**
 * Power can be ON, OFF or undefined.
 * In the later case, we want to display a question mark.
 */
function getPowerLabel(wall: IWallInfo): string {
    if (wall.power) return 'ON'
    if (wall.powerIsUndef) return '?'
    return 'OFF'
}


/**
 * Return stuff like "3 min. ago",
 * or "at 11:42 AM"
 * or "on Feb 13 2019 at 8:12 PM"
 */
function getSmartDate(date: Date): string {
    const now = new Date()
    const ss = (now.getTime() - date.getTime()) * 0.001
    const mm = Math.ceil(ss / 60)

    if (mm < 60) return `${mm} min. ago`
    if (now.getFullYear() === date.getFullYear()
            && now.getMonth() === date.getMonth()
            && now.getDate() === date.getDate()) {
        if (date.getHours() < 13) {
            return `at ${date.getHours()}:${pad(date.getMinutes())} AM`
        }
        else {
            return `at ${date.getHours() - 12}:${pad(date.getMinutes())} PM`
        }
    }
    return `on ${date.toLocaleString()}`
}


function pad(num: number, length: number = 2): string {
    let txt = `${num}`
    while (txt.length < length) {
        txt = `0${txt}`
    }
    return txt
}
