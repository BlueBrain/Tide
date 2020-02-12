import React from "react"

import "./search-button.css"

interface ISearchButtonProps {
    name: string,
    path: string,
    size: number,
    lastModified: string,
    onOpen: (name: string) => void
}

export default class SearchButton extends React.Component<ISearchButtonProps, {}> {
    handleOpen = () => {
        this.props.onOpen(this.props.path)
    }

    render() {
        const { name, size, lastModified }  = this.props
        const classes = ["tideGui-SearchButton", "thm-bg3"]

        return (<div className={classes.join(' ')}
                     onClick={this.handleOpen}>
            <div className='name' title={name}>{name}</div>
            <div className='info'>
                <div>{`${Math.ceil(size / 1024)} Kb`}</div>
                <div>{lastModified}</div>
            </div>
        </div>)
    }
}
