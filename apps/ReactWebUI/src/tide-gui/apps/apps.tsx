import React from "react"
import Button from '../../tfw/view/button'
import Input from '../../tfw/view/input'

import "./apps.css"

interface IAppsProps {
    onOpenBrowser: (url: string) => void,
    onOpenWhiteboard: () => void
}

interface IAppsState {
    url: string
}

export default class Apps extends React.Component<IAppsProps, IAppsState> {
    constructor(props: IAppsProps) {
        super(props)
        this.state = {
            url: ''
        }
    }

    handleUrlChange = (url: string) => {
        this.setState({ url })
    }

    handleOpenBrowser = () => {
        this.props.onOpenBrowser(this.state.url)
    }

    handleOpenWhiteboard = () => {
        this.props.onOpenWhiteboard()
    }

    render() {
        return (<div className="tideGui-Apps">
            <div className="input-row">
                <Input label="Web Browser"
                    value={this.state.url}
                    wide={true}
                    placeholder="Type an URL and press Enter"
                    onChange={this.handleUrlChange}
                    onEnterPressed={this.handleOpenBrowser}/>
                <Button label="Open"
                    icon="web"
                    onClick={this.handleOpenBrowser}/>
            </div>
            <hr/>
            <Button label="Open a whiteboard"
                icon="teach" wide={true}
                onClick={this.handleOpenWhiteboard} />
        </div>)
    }
}
