import React from "react"
import Util from '../../tfw/util'
import Input from '../../tfw/view/input'
import Button from '../../tfw/view/button'
import FileButton from './file-button'
import FilterMatcher from '../filter-matcher'

import "./files.css"

interface IFilesProps {
    // Can be a file or a directory
    onOpen: (uri: string) => void
}

interface IFilesState {
    files: IFile[],
    filter: string
}

export interface IFile {
    dir: boolean,
    name: string
}

/**
 * When we call "tide/files/", we get an array of IFileReponse
 */
interface IFileReponse {
    dir: boolean,
    name: string
}

export default class Files extends React.Component<IFilesProps, IFilesState> {
    private path: string[] = []

    constructor( props: IFilesProps ) {
        super( props );
        this.state = {
            files: [],
            filter: ''
        }
        this.refresh()
    }

    private refresh = async (): Promise<IFile[]> => {
        const { path } = this
        const response: IFileReponse[] =
            await Util.loadJsonFromURL(`${restUrl}files/` + path.join("/"))
        const files = response
            .map((file: IFileReponse): IFile => ({ ...file } as IFile))
            .sort((a: IFile, b: IFile) => {
                if (a.dir === false && b.dir === true) return +1
                if (a.dir === true && b.dir === false) return -1
                if (a.name.toLowerCase() > b.name.toLowerCase() ) return +1
                if (a.name.toLowerCase() < b.name.toLowerCase() ) return -1
                return 0
            })
        this.setState({ files })
        return files
    }

    handleBrowseDir = (dirname: string) => {
        this.path.push(dirname)
        this.refresh()
    }

    handleOpenDir = () => {
        const files = this.state.files
            .filter((file: IFile) => !file.dir)
        if (files.length > 10) {
            swal({
                type: "warning",
                title: "Are you sure?",
                text: "You intend to open a folder with " + files.length + " files.",
                confirmButtonColor: "#DD6B55",
                confirmButtonText: "Yes",
                cancelButtonText: "No",
                closeOnConfirm: true,
                closeOnCancel: true,
                showCancelButton: true
            }, (isConfirm: boolean) => {
                if (!isConfirm) return
                this.props.onOpen(this.path.join('/'))
            });
        } else {
            this.props.onOpen(this.path.join('/'))
        }
    }

    handleOpenFile = (fileName: string) => {
        this.props.onOpen(this.path.join('/') + '/' + fileName)
    }

    handleFilterChange = (filter: string) => {
        this.setState({ filter })
    }

    handleMoveUp = () => {
        if (this.path.length === 0) return;
        this.path.pop()
        this.refresh()
    }

    render() {
        const matcher = new FilterMatcher(this.state.filter)
        const files = this.state.files
            .filter((file: IFile) => matcher.matches(file.name))
        const dirsCount = files.filter((file: IFile) => file.dir).length
        const filesCount = files.length - dirsCount

        return (<div className="tideGuide-Files">
            <nav>
                <Button label="Move up"
                    icon="up"
                    enabled={this.path.length > 0}
                    onClick={this.handleMoveUp}/>
                <Button label={`Open all regular files: ${filesCount}`}
                    icon="show"
                    enabled={filesCount > 0}
                    onClick={this.handleOpenDir}/>
            </nav>
            <header>
                <Input label={`Filter (${dirsCount
                    } director${dirsCount > 1 ? 'ies' : 'y'
                    } and ${filesCount} file${
                        filesCount > 1 ? 's' : ''
                    }):`}
                    focus={true}
                    wide={true}
                    value={this.state.filter}
                    onChange={this.handleFilterChange}/>
            </header>
            <hr/>
            <div>{
                files.map((file: IFile, tabIndex: number) => (
                    <FileButton
                        key={file.name}
                        tabIndex={tabIndex}
                        dir={file.dir}
                        name={file.name}
                        onOpen={this.handleOpenFile}
                        onBrowse={this.handleBrowseDir}/>
                ))
            }</div>
        </div>)
    }
}
