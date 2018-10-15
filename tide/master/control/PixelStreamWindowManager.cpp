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
#include "control/DisplayGroupController.h"
#include "control/WindowController.h"
#include "scene/ContentFactory.h"
#include "scene/ContentType.h"
#include "scene/DisplayGroup.h"
#include "scene/PixelStreamContent.h"
#include "scene/Scene.h"
#include "scene/Window.h"
#include "utils/log.h"

#include <deflect/server/EventReceiver.h>
#include <deflect/server/Frame.h>

namespace
{
const QSize EMPTY_STREAM_SIZE(640, 480);
const uint SURFACE0 = 0;
const uint CHANNEL0 = 0;

WindowPtr _makeStreamWindow(const QString& uri, const QSize& size,
                            const StreamType type, const uint channel)
{
    auto content = ContentFactory::getPixelStreamContent(uri, size, type);
    const auto windowType =
        (type == StreamType::LAUNCHER) ? Window::PANEL : Window::DEFAULT;
    static_cast<PixelStreamContent&>(*content).setChannel(channel);
    return std::make_shared<Window>(std::move(content), windowType);
}

std::set<uint8_t> _findAllChannels(const deflect::server::Frame& frame)
{
    std::set<uint8_t> channels;
    for (const auto& tile : frame.tiles)
        channels.insert(tile.channel);
    return channels;
}

template <typename Windows>
const QUuid& _findWindowIdForChannel(const Windows& windows, const uint channel)
{
    for (const auto& window : windows)
    {
        if (window.second.channel == channel)
            return window.second.uuid;
    }
    throw window_not_found_error("");
}
}

PixelStreamWindowManager::PixelStreamWindowManager(Scene& scene)
    : _scene(scene)
{
    for (auto i = 0u; i < scene.getSurfaceCount(); ++i)
        _monitor(scene.getSurface(i).getGroup(), i);
}

WindowPtrs PixelStreamWindowManager::getWindows(const QString& uri) const
{
    const auto it = _streams.find(uri);
    if (it == _streams.end())
        return WindowPtrs();

    WindowPtrs windows;
    for (const auto& surfaceAndWindowInfo : it->second.windows)
    {
        const auto& windowId = surfaceAndWindowInfo.second.uuid;
        windows.emplace_back(_scene.findWindow(windowId));
    }
    return windows;
}

void PixelStreamWindowManager::hideWindows(const QString& uri)
{
    for (auto&& window : getWindows(uri))
        window->setState(Window::HIDDEN);
}

void PixelStreamWindowManager::showWindows(const QString& uri)
{
    for (auto&& window : getWindows(uri))
        _show(*window);
}

void PixelStreamWindowManager::openWindow(const uint surfaceIndex,
                                          const QString& uri, const QSize& size,
                                          const QPointF& pos,
                                          const StreamType stream)
{
    const auto isLocalStream = stream != StreamType::EXTERNAL;

    if (size.isEmpty() && isLocalStream)
        throw std::logic_error("Window size cannot be empty for local streams");

    if (_isWindowOpen(uri, surfaceIndex) || !_isValid(surfaceIndex))
        return;

    const auto channel = isLocalStream ? CHANNEL0 : surfaceIndex;

    print_log(
        LOG_INFO, LOG_STREAM,
        "opening pixel stream window: '%s' on  surface '%u' for channel '%u'",
        uri.toLocal8Bit().constData(), surfaceIndex, channel);

    auto window = _makeStreamWindow(uri, size, stream, channel);
    auto& group = _scene.getGroup(surfaceIndex);

    WindowController controller{*window, group};
    controller.resize(size.isValid() ? size : EMPTY_STREAM_SIZE);
    controller.moveCenterTo(!pos.isNull() ? pos : group.center());

    window->setState(Window::HIDDEN);
    group.add(window); // triggers _onWindowAdded()

    if (isLocalStream || _isStreamVisible(uri))
        _show(*window);
}

void PixelStreamWindowManager::handleStreamStart(const QString uri)
{
    // Internal streams (or when restored from a session) already have a window.
    // The requestFrame signal from the DataProvider on the wall process has
    // arrived before the Stream was started and ignored by the deflect::Server;
    // hence the first frame must be requested here again.
    if (_streams.count(uri))
    {
        print_log(LOG_INFO, LOG_STREAM,
                  "start sending frames for stream window: '%s'",
                  uri.toLocal8Bit().constData());
        emit requestFirstFrame(uri);
    }
    // External streams don't have a window yet, create one. The first frame
    // will be requested by the DataProvider on the wall process as soon as it's
    // ready to accept them. Emitting requestFirstFrame directly here would be
    // incorrect. It causes a race condition leaving the stream window to be
    // black occasionally (see also DISCL-382 / PR #77).
    else
    {
        openWindow(SURFACE0, uri, QSize());
    }
}

void PixelStreamWindowManager::handleStreamEnd(const QString uri)
{
    if (!_streams.count(uri))
        return;

    print_log(LOG_INFO, LOG_STREAM, "closing pixel stream windows: '%s'",
              uri.toLocal8Bit().constData());

    _windowController.closeAllWindows(uri);
}

void PixelStreamWindowManager::registerEventReceiver(
    const QString uri, const bool exclusive,
    deflect::server::EventReceiver* receiver,
    deflect::server::BoolPromisePtr success)
{
    auto windows = getWindows(uri);
    if (windows.empty())
    {
        print_log(LOG_ERROR, LOG_STREAM,
                  "No window found for %s during registerEventReceiver",
                  uri.toLocal8Bit().constData());
        success->set_value(false);
        return;
    }

    // If a receiver is already registered, don't register this one if
    // "exclusive" was requested
    auto& content = dynamic_cast<PixelStreamContent&>(windows[0]->getContent());
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

void PixelStreamWindowManager::updateStreamWindows(
    deflect::server::FramePtr frame)
{
    if (!_streams.count(frame->uri))
        return;

    auto& windows = _streams.at(frame->uri).windows;
    const auto channels = _findAllChannels(*frame);

    for (const auto channel : channels)
    {
        const auto frameSize = frame->computeDimensions(channel);
        try
        {
            const auto& windowId = _findWindowIdForChannel(windows, channel);
            auto wg = _scene.findWindowAndGroup(windowId);
            _updateWindowSize(wg.first, wg.second, frameSize);
        }
        catch (const window_not_found_error&)
        {
            openWindow(channel, frame->uri, frameSize);
        }
    }

    _closeWindowsWithoutAChannel(frame->uri, channels);
}

void PixelStreamWindowManager::sendDataToWindow(const QString uri,
                                                const QByteArray data)
{
    auto windows = getWindows(uri);
    if (windows.empty())
        return;

    // currently applies to first window only (deflect limitation)
    auto& content = dynamic_cast<PixelStreamContent&>(windows[0]->getContent());
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
    if (!_streams.count(uri))
        return;

    try
    {
        // currently applies to first window only (deflect limitation)
        const auto windowId = _streams.at(uri).windows.begin()->second.uuid;
        auto windowGroup = _scene.findWindowAndGroup(windowId);
        auto& window = windowGroup.first;
        auto& group = windowGroup.second;

        window.getContent().setSizeHints(hints);

        const auto size = QSize(hints.preferredWidth, hints.preferredHeight);
        if (size.isEmpty() || !window.getContent().getDimensions().isEmpty())
            return;

        _updateWindowSize(window, group, size);
    }
    catch (const window_not_found_error&)
    {
    }
}

void PixelStreamWindowManager::_monitor(const DisplayGroup& group,
                                        const uint surfaceIndex)
{
    connect(&group, &DisplayGroup::windowRemoved,
            [this, surfaceIndex](WindowPtr window) {
                _onWindowRemoved(window, surfaceIndex);
            });

    connect(&group, &DisplayGroup::windowAdded,
            [this, surfaceIndex](WindowPtr window) {
                _onWindowAdded(window, surfaceIndex);
            });
}

bool PixelStreamWindowManager::_isWindowOpen(const QString& uri,
                                             const uint surfaceIndex) const
{
    const auto& it = _streams.find(uri);
    return it != _streams.end() && it->second.windows.count(surfaceIndex);
}

bool PixelStreamWindowManager::_isStreamVisible(const QString& uri) const
{
    for (const auto& window : getWindows(uri))
        if (!window->isHidden())
            return true;
    return false;
}

void PixelStreamWindowManager::_show(Window& window)
{
    window.setState(Window::NONE);
    if (getAutoFocusNewWindows())
        _focus(window);
}

bool PixelStreamWindowManager::_isValid(const uint surfaceIndex) const
{
    return surfaceIndex < _scene.getSurfaceCount();
}

void PixelStreamWindowManager::_focus(const Window& window)
{
    auto& group = _scene.findWindowAndGroup(window.getID()).second;
    DisplayGroupController{group}.focus(window.getID());
}

void PixelStreamWindowManager::_onWindowAdded(WindowPtr window,
                                              const uint surfaceIndex)
{
    if (auto content =
            dynamic_cast<const PixelStreamContent*>(window->getContentPtr()))
    {
        // Do the mapping here to include streamers restored from a session (for
        // which openWindow is not called).
        auto& windows = _streams[content->getUri()].windows;
        windows.emplace(surfaceIndex,
                        WindowInfo{window->getID(), content->getChannel()});
    }
}

void PixelStreamWindowManager::_onWindowRemoved(WindowPtr window,
                                                const uint surfaceIndex)
{
    if (!dynamic_cast<const PixelStreamContent*>(window->getContentPtr()))
        return;

    const auto& uri = window->getContent().getUri();

    auto& windows = _streams.at(uri).windows;
    windows.erase(surfaceIndex);
    if (windows.empty())
    {
        _streams.erase(uri);
        emit streamWindowClosed(uri);
    }
}

void PixelStreamWindowManager::_closeWindowsWithoutAChannel(
    const QString& uri, const std::set<uint8_t>& channels)
{
    std::vector<QUuid> windowsToRemove;
    for (const auto& window : _streams.at(uri).windows)
        if (!channels.count(window.second.channel))
            windowsToRemove.push_back(window.second.uuid);

    for (const auto& windowId : windowsToRemove)
        _windowController.closeSingleWindow(uri, windowId);
}

void PixelStreamWindowManager::_updateWindowSize(Window& window,
                                                 DisplayGroup& group,
                                                 const QSize& size)
{
    auto& content = window.getContent();

    if (size == content.getDimensions())
        return;

    const auto externalStreamIsOpening = content.getDimensions().isEmpty();
    content.setDimensions(size);

    if (externalStreamIsOpening)
        _resizeInPlace(window, group, size);

    // Must come after window resize; window size is used by focus algorithm
    if (window.isFocused())
        DisplayGroupController{group}.updateFocusedWindowsCoordinates();

    // ready to make visible
    if (externalStreamIsOpening)
        emit externalStreamOpening(content.getUri());
}

void PixelStreamWindowManager::_resizeInPlace(Window& window,
                                              const DisplayGroup& group,
                                              const QSize& size)
{
    const auto target = WindowController::Coordinates::STANDARD;
    WindowController controller{window, group, target};
    controller.resize(size, CENTER);
}
