import React from "react"
import Util from '../../tfw/util'
import Input from '../../tfw/view/input'
import Button from '../../tfw/view/button'
import SessionButton from './session-button'
import FilterMatcher from '../filter-matcher'
import Storage from '../../tfw/storage'

import "./sessions.css"

interface ISessionsProps {
    onOpen: (sessionName: string) => void
    onSave: (sessionName: string, overwrite: boolean) => void
}

interface ISessionsState {
    // Bookmarked sessions will appear before the other.
    bookmarks: string[],
    sessionNames: string[],
    filter: string,
    nameOfSessionToSave: string
}

interface ISession {
    bookmarked: boolean,
    name: string
}

/**
 * When we call "tide/sessions/", we get an array of ISessionReponse
 */
interface ISessionReponse {
    dir: boolean,
    name: string
}

export default class Sessions extends React.Component<ISessionsProps, ISessionsState> {
    constructor( props: ISessionsProps ) {
        super( props );
        this.state = {
            bookmarks: Storage.local.get("Tide/sessions/bookmark", []),
            sessionNames: [],
            filter: '',
            nameOfSessionToSave: ''
        }
        this.refresh()
    }

    private async refresh() {
        const response: ISessionReponse[] =
            await Util.loadJsonFromURL(`${restUrl}sessions/`)
        const sessionNames = response
            .filter((session: ISessionReponse) => session.dir === false)
            .map((session: ISessionReponse) => session.name)
        this.setState({ sessionNames })
        return sessionNames
    }

    handleSessionToSaveChange = (name: string) => {
        this.setState({
            nameOfSessionToSave: name
        })
    }

    handleBookmarkSession = (name: string, bookmarked: boolean) => {
        const bookmarks = this.state.bookmarks.filter((bookmark: string) => (
            bookmark !== name
        ))
        if (bookmarked) bookmarks.push(name)
        this.setState({ bookmarks })
        Storage.local.set("Tide/sessions/bookmark", bookmarks)
    }

    handleOpenSession = (name: string) => {
        this.props.onOpen(name)
    }

    handleSaveSession = async () => {
        let name = this.state.nameOfSessionToSave
        if (!name.endsWith('.dcx')) name += '.dcx'
        const sessionNames = await this.refresh()
        console.info("sessionNames, name=", sessionNames, name);
        const overwrite = sessionNames.indexOf(name) !== -1
        this.props.onSave(name, overwrite)
    }

    handleFilterChange = (filter: string) => {
        this.setState({ filter })
    }

    render() {
        const matcher = new FilterMatcher(this.state.filter)
        const bookmarks = this.state.bookmarks
        const sessions = this.state.sessionNames
            .filter(matcher.matches.bind(matcher))
            .map((sessionName: string) => {
                const isBookmarked = bookmarks.indexOf(sessionName)
                const prefix = isBookmarked !== -1 ? "0" : "1"
                return `${prefix}${sessionName}`
            })
            .sort(insensitiveSort)
            .map((prefixedSessionName: string) => ({
                bookmarked: prefixedSessionName.charAt(0) === '0',
                name: prefixedSessionName.substr(1)
            }))

        return (<div className="tideGuide-Sessions">
            <header>
                <Input label="Filter on session names"
                    focus={true}
                    wide={true}
                    value={this.state.filter}
                    onChange={this.handleFilterChange}/>
            </header>
            <div>{
                sessions.map((session: ISession) => (
                    <SessionButton
                        key={session.name}
                        name={session.name}
                        bookmarked={session.bookmarked}
                        onOpen={this.handleOpenSession}
                        onBookmark={this.handleBookmarkSession}/>
                ))
            }</div>
            <hr/>
            <footer>
                <Input focus={false}
                    wide={true}
                    label="Session's Name"
                    value={this.state.nameOfSessionToSave}
                    onEnterPressed={this.handleSaveSession}
                    onChange={this.handleSessionToSaveChange}/>
                <Button
                    label="Save session"
                    onClick={this.handleSaveSession}
                    enabled={this.state.nameOfSessionToSave.trim().length > 0}
                    icon="export"/>
            </footer>
        </div>)
    }
}


function insensitiveSort(a: string, b: string) {
    const A = `${a}`.trim().toUpperCase()
    const B = `${b}`.trim().toUpperCase()
    if (A < B) return -1
    if (A > B) return +1
    return 0
}
