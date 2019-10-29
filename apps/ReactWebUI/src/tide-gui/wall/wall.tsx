import React from "react"
import Icon from '../../tfw/view/icon'
import Touchable from '../../tfw/view/touchable'
import QueryService, { IWallInfo } from '../service/query'

import "./wall.css"

interface IWallProps {
    wall?: number,
    onClick: (wallNumber: number) => void
}

interface IWallState {
    walls: IWallInfo[]
}

export default class Wall extends React.Component<IWallProps, IWallState> {
    private intervalId: number = 0

    constructor(props: IWallProps) {
        super(props)
        this.state = {
            walls: []
        }
    }

    componentDidMount() {
        this.intervalId = window.setInterval(this.refreshWallsInfo, 1000)
    }

    componentWillUnmount() {
        window.clearInterval(this.intervalId)
        this.intervalId = 0
    }

    private refreshWallsInfo = async () => {
        const walls = await QueryService.getWallsStatus()
        this.setState({ walls })
    }

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
            <div className="icon">
                <div className={wall.power ? 'power-on' : 'power-off'}>
                    <Icon content="tv" size={28} pen0="#0f0"/>
                </div>
                <div className={wall.power ? 'power-on' : 'power-off'}>
                    <Icon content="tv" size={28} pen0="#0f0"/>
                </div>
                <div>
                    <Icon content="tv" size={28}/>
                </div>
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
        return (<div className="tideGui-Wall">
            <h1>Please select a display wall</h1>
            <div>{
                this.state.walls.map(this.renderWall)
            }</div>
        </div>)
    }
}
