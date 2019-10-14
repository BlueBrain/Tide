import React from "react"
import Touchable from '../../tfw/view/touchable'

import "./wall.css"
import Wall6 from './wall-06.jpg'
import Wall5 from './wall-05.jpg'
import Wall0 from './wall-00.jpg'

interface IWallProps {
    wall?: number,
    onClick: (wallNumber: number) => void
}

export default class Wall extends React.Component<IWallProps, {}> {
    renderWall(index: number) {
        const disabledIndex = this.props.wall
        let src = Wall0
        if (index === 5) src = Wall5
        else if (index === 6) src = Wall6

        const className = disabledIndex === index ? 'wall disabled' : 'wall thm-ele-nav'
        return <Touchable
                    enabled={index !== disabledIndex}
                    onClick={() => this.props.onClick(index)}>
            <img className={className} src={src}/>
        </Touchable>
    }

    render() {
        return (<div className="tideGui-Wall">
            <h1>Please select a display wall</h1>
            <div>
                {this.renderWall(6)}
                {this.renderWall(5)}
                {this.renderWall(0)}
            </div>
        </div>)
    }
}
