/**
 * Here is a React strapping on a vanilla JS app.
 * All the vanilla JS stands in "/public/js" folder.
 *
 * Here, we add events on few menu buttons.
 * We also start a pulling on walls status.
 * This status is pushed to the global App State (in Redux way).
 */

import React from 'react';
import FilesView from './tide-gui/files'
import AppsView from './tide-gui/apps'
import WallView from './tide-gui/wall'
import SearchView from './tide-gui/search'
import SessionsView from './tide-gui/sessions'
import ClipboardView from './tide-gui/clipboard'
import Theme from './tfw/theme'
import Dialog from './tfw/factory/dialog'
import Button from './tfw/view/button'
import Gesture from './tfw/gesture'
import castInteger from './tfw/converter/integer'
import Args from './tfw/url-args'
import { Provider } from 'react-redux'
import State from './tide-gui/state'
import QueryService from './tide-gui/service/query'

import * as serviceWorker from './serviceWorker';

import './index.css'

const WALL_REFRESH_INTERVAL = 5000

const appsButton = document.getElementById('appsButton2')
const wallButton = document.getElementById('wallButton')
const fileButton = document.getElementById('fileButton')
const searchButton = document.getElementById('searchButton')
const sessionButton = document.getElementById('sessionButton2')
const clipboardButton = document.getElementById('clipboardButton')

const wallIndex = castInteger(Args.parse().wall, -1)

async function start() {
    Theme.register("default", {
        white: "#eef", black: "#000",
        bg0: "#ccc", bg3: "#fff",
        bgP: "#014f86",
        bgS: "#ff8800"
    });
    Theme.apply("default");

    // Check if the "wall" argument has been set correctly.
    if (wallIndex === -1) {
        showWallMenu(false)
        return
    }

    if (wallButton) {
        Gesture(wallButton).on({
            down: () => showWallMenu(true)
        })
    }

    if (appsButton) {
        Gesture(appsButton).on({
            down: showAppMenu
        })
    }

    if (sessionButton) {
        Gesture(sessionButton).on({
            down: showSessionMenu
        })
    }

    if (fileButton) {
        Gesture(fileButton).on({
            down: showFileMenu
        })
    }

    if (searchButton) {
        Gesture(searchButton).on({
            down: showSearchMenu
        })
    }

    if (clipboardButton) {
        Gesture(clipboardButton).on({
            down: showClipboardMenu
        })
    }

    await refreshWalls()
}

start()

async function refreshWalls() {
    const walls = await QueryService.getWallsStatus()
    State.dispatch(State.Walls.reset(walls))
}

// If you want your app to work offline and load faster, you can change
// unregister() to register() below. Note this comes with some pitfalls.
// Learn more about service workers: https://bit.ly/CRA-PWA
serviceWorker.unregister();


function showSessionMenu() {
    const onOpen = (name: string) => {
        dialog.hide()
        sendAppJsonRpc("load", { uri: name }, updateWall)
    }

    const onSave = async (name: string, overwrite: boolean) => {
        if (overwrite) {
            swal({
                type: "warning",
                title: "Confirm Session Overwrite",
                text: `Warning! This session already exist: "${name}"`,
                confirmButtonColor: "#DD6B55",
                confirmButtonText: "Yes",
                cancelButtonText: "No",
                closeOnConfirm: true,
                closeOnCancel: true,
                showCancelButton: true
            }, function(isConfirm) {
                if (isConfirm) {
                    dialog.hide()
                    sendAppJsonRpc("save", { uri: name }, () => {
                        swal({
                            type: "success",
                            title: "Saved!",
                            text: "Your session has been saved as: " + name,
                            confirmButtonText: "OK",
                            confirmButtonColor: "#014f86"
                        })
                    })
                }
            });
        }
    }

    const dialog = Dialog.show({
        closeOnEscape: true,
        target: sessionButton,
        width: 480,
        maxWidth: 480,
        content: <SessionsView onOpen={onOpen} onSave={onSave} />,
        footer: <Button flat={true}
                    label="Close (you can also press ESC)" icon="close"
                    onClick={() => dialog.hide()} />
    })
}

function showFileMenu() {
    const onOpen = (uri: string) => {
        sendAppJsonRpc("open", { uri }, updateWall)
    }

    const dialog = Dialog.show({
        closeOnEscape: true,
        target: fileButton,
        width: 480,
        maxWidth: 480,
        content: <FilesView onOpen={onOpen} />,
        footer: <Button flat={true}
                    label="Close (you can also press ESC)" icon="close"
                    onClick={() => dialog.hide()} />
    })
}

/**
 * Button WALL
 */
function showWallMenu(closable: boolean) {
    const intervalId = window.setInterval(refreshWalls, WALL_REFRESH_INTERVAL)

    const view = (
        <Provider store={State.store}>
            <WallView
                wall={wallIndex}
                onClick={(wall: number) => window.location.href = `?wall=${wall}`} />
        </Provider>
    )
    console.info("closable=", closable);
    const dialog = Dialog.show({
        closeOnEscape: closable,
        onClose: () => {
            window.clearInterval(intervalId)
        },
        target: wallButton || undefined,
        width: 480,
        maxWidth: 480,
        content: view,
        footer: closable ? <Button flat={true}
                    label="Close (you can also press ESC)" icon="close"
                    onClick={() => dialog.hide()} /> : null
    })
}

function showSearchMenu() {
    const onOpen = (uri: string) => {
        dialog.hide()
        sendAppJsonRpc("open", { uri }, updateWall)
    }

    const dialog = Dialog.show({
        closeOnEscape: true,
        target: searchButton || undefined,
        width: 480,
        maxWidth: 480,
        content: <SearchView onOpen={onOpen} />,
        footer: <Button flat={true}
                    label="Close (you can also press ESC)" icon="close"
                    onClick={() => dialog.hide()} />
    })
}

function showAppMenu() {
    const onOpenBrowser = (uri: string) => {
        dialog.hide()
        sendAppJsonRpc("browse", { uri, surfaceIndex: 0 }, updateWall)
    }

    const onOpenWhiteboard = () => {
        dialog.hide()
        sendAppJsonRpc("whiteboard", { surfaceIndex: 0 }, updateWall)
    }

    const dialog = Dialog.show({
        closeOnEscape: true,
        target: appsButton,
        width: 480,
        maxWidth: 480,
        content: <AppsView onOpenBrowser={onOpenBrowser}
                           onOpenWhiteboard={onOpenWhiteboard}/>,
        footer: <Button flat={true}
                    label="Close (you can also press ESC)" icon="close"
                    onClick={() => dialog.hide()} />
    })
}

function showClipboardMenu() {
    const onOpenFile = (uri: string) => {
        sendAppJsonRpc("open", { uri }, updateWall)
    }

    const dialog = Dialog.show({
        closeOnEscape: true,
        target: clipboardButton || undefined,
        width: 480,
        maxWidth: 480,
        content: <ClipboardView onOpenFile={onOpenFile}/>,
        footer: <Button flat={true}
                    label="Close (you can also press ESC)" icon="close"
                    onClick={() => dialog.hide()} />
    })
}
