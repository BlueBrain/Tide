import React from "react"
import Util from '../../tfw/util'
import Input from '../../tfw/view/input'
import SearchButton from './search-button'
import Button from '../../tfw/view/button'
import Icon from '../../tfw/view/icon'
import Storage from '../../tfw/storage'

import "./search.css"

interface ISearchsProps {
    onOpen: (uri: string) => void
}

interface ISearchsState {
    filter: string,
    files: ISearchResponse[],
    searching: boolean
}

function storageGet(key: string, defaultValue: any) {
    return Storage.session.get(`tide-gui/search/${key}`, defaultValue)
}

function storageSet(key: string, value: any) {
    Storage.session.set(`tide-gui/search/${key}`, value)
}

/**
 * When we call "tide/searchs/", we get an array of ISearchResponse
 */
export interface ISearchResponse {
    idDir: boolean,
    name: string,
    path: string,
    lastModified: string,
    size: number
}

export default class Searchs extends React.Component<ISearchsProps, ISearchsState> {
    constructor( props: ISearchsProps ) {
        super( props );
        this.state = {
            filter: storageGet('filter', ''),
            files: storageGet('files', []),
            searching: false
        }
    }

    private search = async () => {
        const filter = this.state.filter.trim()
        if (filter.length < 3) return
        this.setState({ searching: true, files: [] })
        const encodedFilter = encodeURIComponent(filter)
        const response: ISearchResponse[] =
            (await Util.loadJsonFromURL(`tide/find/?file=${encodedFilter}`)) || []
        const files = response
            .filter((file: ISearchResponse) => !file.idDir)
            .sort((a: ISearchResponse, b: ISearchResponse) => {
                if (a.name.toLowerCase() > b.name.toLowerCase() ) return +1
                if (a.name.toLowerCase() < b.name.toLowerCase() ) return -1
                return 0
            })
        this.setState({ files, searching: false })
        storageSet("filter", filter)
        storageSet('files', files)
    }

    handleOpenSearch = (searchName: string) => {
        this.props.onOpen(searchName)
    }

    handleFilterChange = (filter: string) => {
        this.setState({ filter })
    }

    render() {
        const { files, filter } = this.state
        return (<div className="tideGuide-Searchs">
            <header>
                <Input label="Search for file:"
                    placeholder="Press Enter to start searching..."
                    focus={true}
                    wide={true}
                    value={filter}
                    enabled={!this.state.searching}
                    onChange={this.handleFilterChange}
                    onEnterPressed={this.search}/>
                <Button label="Search"
                    icon="search"
                    enabled={!this.state.searching}
                    onClick={this.search}/>
            </header>
            <hr/>
            {
                !this.state.searching ? null :
                <div className="wait">
                    <Icon content="wait" animate={true}/>
                    <div>Searching...</div>
                </div>
            }
            <div>{
                files.map((file: ISearchResponse) => (
                    <SearchButton
                        key={file.name}
                        name={file.name}
                        path={file.path}
                        size={file.size}
                        lastModified={file.lastModified}
                        onOpen={this.handleOpenSearch}/>
                ))
            }</div>
        </div>)
    }
}
