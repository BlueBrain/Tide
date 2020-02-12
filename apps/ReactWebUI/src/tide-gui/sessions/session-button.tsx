/**
 * A button that is an item in the sessions list.
 */

import React from "react"
import Icon from '../../tfw/view/icon'

import "./session-button.css"

interface ISessionButtonProps {
    bookmarked: boolean,
    name: string,
    onOpen: (name: string) => void,
    onBookmark: (name: string, bookmarked: boolean) => void
}

export default class SessionButton extends React.Component<ISessionButtonProps, {}> {
    handleOpen = () => {
        this.props.onOpen(this.props.name)
    }

    handleBookmark = () => {
        this.props.onBookmark(this.props.name, !this.props.bookmarked)
    }

    render() {
        const name  = removeFileSuffix(this.props.name)
        return (<div className="tideGui-SessionButton thm-bg2">
            <div className='name' onClick={this.handleOpen} title={name}>{
                name
            }</div>
            <Icon content="star"
                pen0={this.props.bookmarked ? "S" : "0"}
                onClick={this.handleBookmark} />
        </div>)
    }
}


/**
 * All session file names should end with ".dcx".
 * There is no point to display this for every button.
 */
function removeFileSuffix(name: string): string {
    if (name.endsWith('.dcx')) {
        return name.substr(0, name.length - '.dcx'.length)
    }
    return name
}
