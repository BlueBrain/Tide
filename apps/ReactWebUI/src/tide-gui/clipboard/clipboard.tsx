import React from "react"
import Storage from '../../tfw/storage'
import Button from '../../tfw/view/button'
import Touchable from '../../tfw/view/touchable'
import Util from '../../tfw/util'

import "./clipboard.css"

interface IAppsProps {
    onOpenFile: (url: string) => void
}

interface IClipboardItem {
    id: string,
    name: string,
    url: string,
    // Size of the preview
    width: number,
    height: number,
    // DataURI
    image: string
}

interface IAppsState {
    items: IClipboardItem[],
    selection: IWindow[]
}

interface IWindow {
    aspectRatio: number,
    focus: boolean,
    fullscreen: boolean,
    height: number,
    minHeight: number,
    minWidth: number,
    mode: number,
    selected: boolean,
    title: string,
    uri: string,
    uuid: string,
    visible: boolean,
    width: number,
    x: number,
    y: number
}

export default class Apps extends React.Component<IAppsProps, IAppsState> {
    constructor(props: IAppsProps) {
        super(props)
        this.state = {
            items: storageGet("items", []),
            selection: []
        }
    }

    async componentDidMount() {
        const response: {windows: IWindow[]}[] =
            await Util.loadJsonFromURL("tide/windows")
        if (!response || response.length === 0) return
        const windows = response[0].windows
        if (!windows) return

        this.setState({
            selection: windows.filter((w: IWindow) => w.selected)
        })
    }

    /**
     * Persist clipboard content into the storage.
     */
    save = () => {
        storageSet("items", this.state.items)
    }

    async add(newItem: IClipboardItem) {
        const { width, height } = fit(newItem.width, newItem.height)
        newItem.width = width
        newItem.height = height

        const items = this.state.items.filter(
            (item: IClipboardItem) => item.url !== newItem.url
        )
        items.unshift(newItem)
        this.setState({ items }, this.save)

        // Loading preview image.
        const response = await fetch(`tide/windows/${newItem.id}/thumbnail`)
        const img = new Image()
        img.src = await response.text()
        img.onload = () => {
            const canvas = document.createElement("canvas")
            canvas.width = width
            canvas.height = height
            const ctx = canvas.getContext("2d")
            if (!ctx) return
            ctx.drawImage(img, 0, 0, width, height)
            const dataURL = canvas.toDataURL("png")
            this.updateItem(newItem.id, dataURL)
        }
        img.onerror = (err) => {
            console.error("Error in ", newItem.name, img.src)
            console.error(err)
        }
    }

    updateItem(id: string, dataURL: string) {
        console.info("id, dataURL=", id, dataURL);
        const items = this.state.items.slice()
        const item = items.find((i: IClipboardItem) => i.id === id)
        if (!item) return
        item.image = dataURL
        this.setState({ items }, this.save)
    }

    handleCopyClick = () => {
        const { selection } = this.state
        selection.forEach((w: IWindow) => {
            this.add({
                id: w.uuid,
                name: w.title,
                url: w.uri,
                width: w.width,
                height: w.height,
                image: ''
            })
        })
    }

    handlePreviewClick = (url: string) => {
        this.props.onOpenFile(url)
    }

    handleRemoveClick = (url: string) => {
        const items = this.state.items.filter(
            (item: IClipboardItem) => item.url !== url
        )
        this.setState({ items }, this.save)
    }

    render() {
        const { items, selection } = this.state

        return (<div className="tideGui-Clipboard">
            <Button label={`Add current selection to the clipboard (${
                selection.length
            } items)`}
                icon="copy" wide={true}
                enabled={selection.length > 0}
                onClick={this.handleCopyClick}/>
            <hr/>
            <div className="previews">{
                items.length === 0 ? <div>
                    <p>Your clipboard is empty!</p>
                    <p>Select one or many windows on your screen,
                    then use the button above to add the selection
                    to the clipboard.</p>
                </div> :
                items.map((item: IClipboardItem) => (
                    <Touchable
                            classNames={["preview", "thm-ele-nav", 'thm-black']}
                            key={item.url}
                            title={item.name}
                            onClick={() => this.handlePreviewClick(item.url)}>
                        <img src={item.image} style={{
                            width: `${item.width}px`,
                            height: `${item.height}px`
                        }}/>
                        <Button
                            icon="close"
                            warning={true}
                            small={true}
                            onClick={() => this.handleRemoveClick(item.url)}/>
                    </Touchable>
                ))
            }</div>
            {
                items.length === 0 ? null :
                <div className="hint">
                    Click on an image above to add it to the current screen.
                </div>
            }
        </div>)
    }
}


function storageGet(key: string, defaultValue: any = undefined) {
    return Storage.local.get(`tide-gui/clipboard/${key}`, defaultValue)
}

function storageSet(key: string, value: any) {
    Storage.local.set(`tide-gui/clipboard/${key}`, value)
}

function fit(width: number, height: number, frame: number = 120): {width: number, height: number} {
    const factor = width > height ? frame / width : frame / height
    return {
        width: factor * width,
        height: factor * height
    }
}
