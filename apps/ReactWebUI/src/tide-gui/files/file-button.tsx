import React from "react"
import Icon from '../../tfw/view/icon'

import "./file-button.css"

interface IFileButtonProps {
    dir: boolean,
    name: string,
    tabIndex: number,
    onOpen: (name: string) => void,
    onBrowse: (name: string) => void
}

export default class FileButton extends React.Component<IFileButtonProps, {}> {
    handleOpen = () => {
        this.props.onOpen(this.props.name)
    }

    handleBrowse = () => {
        this.props.onBrowse(this.props.name)
    }

    render() {
        const { dir, name, tabIndex }  = this.props
        const classes = ["tideGui-FileButton"]
        if (dir) classes.push("dir", "thm-bg2")
        else classes.push("thm-bg3")

        return (<div className={classes.join(' ')}
                     tabIndex={tabIndex}
                     onClick={dir ? this.handleBrowse : this.handleOpen}>
            <Icon content={dir ? 'folder' : 'file'} />
            <div className='name' title={name}>{
                name
            }</div>
        </div>)
    }
}
