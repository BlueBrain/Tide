/*********************************************************************/
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
/*                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>*/
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/*   1. Redistributions of source code must retain the above         */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer.                                                  */
/*                                                                   */
/*   2. Redistributions in binary form must reproduce the above      */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer in the documentation and/or other materials       */
/*      provided with the distribution.                              */
/*                                                                   */
/*    THIS  SOFTWARE IS PROVIDED  BY THE  UNIVERSITY OF  TEXAS AT    */
/*    AUSTIN  ``AS IS''  AND ANY  EXPRESS OR  IMPLIED WARRANTIES,    */
/*    INCLUDING, BUT  NOT LIMITED  TO, THE IMPLIED  WARRANTIES OF    */
/*    MERCHANTABILITY  AND FITNESS FOR  A PARTICULAR  PURPOSE ARE    */
/*    DISCLAIMED.  IN  NO EVENT SHALL THE UNIVERSITY  OF TEXAS AT    */
/*    AUSTIN OR CONTRIBUTORS BE  LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL,  SPECIAL, EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES    */
/*    (INCLUDING, BUT  NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE    */
/*    GOODS  OR  SERVICES; LOSS  OF  USE,  DATA,  OR PROFITS;  OR    */
/*    BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF    */
/*    LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*    (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT    */
/*    OF  THE  USE OF  THIS  SOFTWARE,  EVEN  IF ADVISED  OF  THE    */
/*    POSSIBILITY OF SUCH DAMAGE.                                    */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#include "PixelStreamWindowManager.h"

#include "control/ContentController.h"
#include "control/ContentWindowController.h"
#include "control/DisplayGroupController.h"
#include "localstreamer/PixelStreamerLauncher.h"
#include "log.h"
#include "scene/ContentFactory.h"
#include "scene/ContentType.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"
#include "scene/PixelStreamContent.h"
#include "scene/Scene.h"

#include <deflect/server/EventReceiver.h>
#include <deflect/server/Frame.h>

namespace
{
const QSize EMPTY_STREAM_SIZE(640, 480);

bool _isLauncher(const QString& uri)
{
    return uri == PixelStreamerLauncher::launcherUri;
}

ContentWindowPtr _makeStreamWindow(const QString& uri, const QSize& size,
                                   const StreamType stream)
{
    auto content = ContentFactory::getPixelStreamContent(uri, stream);

    if (size.isValid())
        content->setDimensions(size);

    const auto type = (stream == StreamType::LAUNCHER) ? ContentWindow::PANEL
                                                       : ContentWindow::DEFAULT;
    return std::make_shared<ContentWindow>(std::move(content), type);
}
}

PixelStreamWindowManager::PixelStreamWindowManager(Scene& scene)
    : _scene(scene)
{
    for (const auto& surface : scene.getSurfaces())
        _monitor(surface.getGroup());
}

ContentWindowPtr PixelStreamWindowManager::getWindow(const QString& uri) const
{
    const auto it = _streamWindows.find(uri);
    if (it != _streamWindows.end())
        return _scene.findWindow(it->second);

    return ContentWindowPtr();
}

void PixelStreamWindowManager::hideWindow(const QString& uri)
{
    if (auto window = getWindow(uri))
        window->setState(ContentWindow::HIDDEN);
}

void PixelStreamWindowManager::showWindow(const QString& uri)
{
    if (!_streamWindows.count(uri))
        return;

    auto windowGroup = _scene.findWindowAndGroup(_streamWindows.at(uri));
    auto& window = windowGroup.first;
    auto& group = windowGroup.second;

    window.setState(ContentWindow::NONE);
    if (_autoFocusNewWindows)
        DisplayGroupController{group}.focus(window.getID());
}

void PixelStreamWindowManager::openWindow(const uint surfaceIndex,
                                          const QString& uri,
                                          const QPointF& pos, const QSize& size,
                                          const StreamType stream)
{
    if (_isWindowOpen(uri) || surfaceIndex >= _scene.getSurfaces().size())
        return;

    print_log(LOG_INFO, LOG_STREAM, "opening pixel stream window: '%s'",
              uri.toLocal8Bit().constData());

    auto window = _makeStreamWindow(uri, size, stream);
    auto& group = _scene.getGroup(surfaceIndex);

    ContentWindowController controller{*window, group};
    controller.resize(size.isValid() ? size : EMPTY_STREAM_SIZE);
    controller.moveCenterTo(!pos.isNull() ? pos : group.center());

    window->setState(ContentWindow::HIDDEN);
    group.addContentWindow(window);

    // external streams are shown once we receive proper sizes
    if (stream != StreamType::EXTERNAL)
        showWindow(uri);
}

void PixelStreamWindowManager::handleStreamStart(const QString uri)
{
    // internal streams already have a window
    if (_isWindowOpen(uri))
    {
        emit requestFirstFrame(uri);
        print_log(LOG_INFO, LOG_STREAM,
                  "start sending frames for stream window: '%s'",
                  uri.toLocal8Bit().constData());
        return;
    }

    // external streams and launcher don't have a window yet, create one now
    const auto streamType =
        (_isLauncher(uri)) ? StreamType::LAUNCHER : StreamType::EXTERNAL;
    openWindow(0, uri, QPointF(), QSize(), streamType);
}

void PixelStreamWindowManager::handleStreamEnd(const QString uri)
{
    print_log(LOG_INFO, LOG_STREAM, "closing pixel stream window: '%s'",
              uri.toLocal8Bit().constData());
    try
    {
        if (!_streamWindows.count(uri))
            return;

        auto windowGroup = _scene.findWindowAndGroup(_streamWindows.at(uri));
        const auto& window = windowGroup.first;
        auto& group = windowGroup.second;
        DisplayGroupController{group}.remove(window.getID());
    }
    catch (const window_not_found_error&)
    {
    }
}

void PixelStreamWindowManager::registerEventReceiver(
    const QString uri, const bool exclusive,
    deflect::server::EventReceiver* receiver,
    deflect::server::BoolPromisePtr success)
{
    auto window = getWindow(uri);
    if (!window)
    {
        print_log(LOG_ERROR, LOG_STREAM,
                  "No window found for %s during registerEventReceiver",
                  uri.toLocal8Bit().constData());
        success->set_value(false);
        return;
    }

    // If a receiver is already registered, don't register this one if
    // "exclusive" was requested
    auto& content = dynamic_cast<PixelStreamContent&>(window->getContent());
    if (!exclusive || !content.hasEventReceivers())
    {
        if (connect(&content, &PixelStreamContent::notify, receiver,
                    &deflect::server::EventReceiver::processEvent))
        {
            content.incrementEventReceiverCount();
            success->set_value(true);
            return;
        }
        print_log(LOG_ERROR, LOG_STREAM, , "QObject connection failed");
    }
    success->set_value(false);
}

void PixelStreamWindowManager::updateStreamDimensions(
    deflect::server::FramePtr frame)
{
    try
    {
        const auto& uri = frame->uri;
        if (!_streamWindows.count(uri))
            return;

        auto windowGroup = _scene.findWindowAndGroup(_streamWindows.at(uri));
        auto& window = windowGroup.first;
        auto& group = windowGroup.second;
        const auto size = frame->computeDimensions();
        _updateWindowSize(window, group, size);
    }
    catch (const window_not_found_error&)
    {
    }
}

void PixelStreamWindowManager::sendDataToWindow(const QString uri,
                                                const QByteArray data)
{
    auto window = getWindow(uri);
    if (!window)
        return;

    auto& content = dynamic_cast<PixelStreamContent&>(window->getContent());
    content.parseData(data);
}

bool PixelStreamWindowManager::getAutoFocusNewWindows() const
{
    return _autoFocusNewWindows;
}

void PixelStreamWindowManager::setAutoFocusNewWindows(const bool set)
{
    _autoFocusNewWindows = set;
}

void PixelStreamWindowManager::updateSizeHints(const QString uri,
                                               const deflect::SizeHints hints)
{
    try
    {
        if (!_streamWindows.count(uri))
            return;

        auto windowGroup = _scene.findWindowAndGroup(_streamWindows.at(uri));
        auto& window = windowGroup.first;
        auto& group = windowGroup.second;

        window.getContent().setSizeHints(hints);

        const QSize size(hints.preferredWidth, hints.preferredHeight);
        if (size.isEmpty() || !window.getContent().getDimensions().isEmpty())
            return;

        _updateWindowSize(window, group, size);
    }
    catch (const window_not_found_error&)
    {
    }
}

void PixelStreamWindowManager::_monitor(const DisplayGroup& group)
{
    connect(&group, &DisplayGroup::contentWindowRemoved, this,
            &PixelStreamWindowManager::_onWindowRemoved);
    connect(&group, &DisplayGroup::contentWindowAdded, this,
            &PixelStreamWindowManager::_onWindowAdded);
}

bool PixelStreamWindowManager::_isWindowOpen(const QString& uri) const
{
    return _streamWindows.find(uri) != _streamWindows.end();
}

bool _isStreamType(const CONTENT_TYPE type)
{
    return type == CONTENT_TYPE_PIXEL_STREAM || type == CONTENT_TYPE_WEBBROWSER;
}

void PixelStreamWindowManager::_onWindowAdded(ContentWindowPtr window)
{
    // Do the mapping here, not in openWindow(), to include any streamer
    // restored from a session (for which openWindow is not called).
    if (_isStreamType(window->getContent().getType()))
        _streamWindows[window->getContent().getURI()] = window->getID();
}

void PixelStreamWindowManager::_onWindowRemoved(ContentWindowPtr window)
{
    if (!_isStreamType(window->getContent().getType()))
        return;

    const auto& uri = window->getContent().getURI();
    _streamWindows.erase(uri);
    emit streamWindowClosed(uri);
}

void PixelStreamWindowManager::_updateWindowSize(ContentWindow& window,
                                                 DisplayGroup& group,
                                                 const QSize size)
{
    bool emitOpen = false;

    // External streamers might not have reported an initial size yet
    if (window.getContent().getDimensions().isEmpty())
    {
        emitOpen = true;

        const auto target = ContentWindowController::Coordinates::STANDARD;
        ContentWindowController controller{window, group, target};
        controller.resize(size, CENTER);
    }

    if (size == window.getContent().getDimensions())
        return;

    window.getContent().setDimensions(size);
    if (window.isFocused())
        DisplayGroupController{group}.updateFocusedWindowsCoordinates();

    if (emitOpen)
        emit externalStreamOpening(window.getContent().getURI());
}
