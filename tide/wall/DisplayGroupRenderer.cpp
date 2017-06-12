/*********************************************************************/
/* Copyright (c) 2014-2017, EPFL/Blue Brain Project                  */
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

#include "DisplayGroupRenderer.h"

#include "DataProvider.h"
#include "InactivityTimer.h"
#include "VisibilityHelper.h"
#include "WallWindow.h"
#include "geometry.h"
#include "qmlUtils.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"
#include "scene/Markers.h"
#include "scene/Options.h"

#include <deflect/Frame.h>

#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>

namespace
{
const QUrl QML_DISPLAYGROUP_URL("qrc:/qml/wall/WallDisplayGroup.qml");
}

DisplayGroupRenderer::DisplayGroupRenderer(WallWindow& parentWindow,
                                           DataProvider& provider,
                                           const QRect& screenRect,
                                           const deflect::View view)
    : _engine{*parentWindow.engine()}
    , _provider{provider}
    , _displayGroup{new DisplayGroup(QSize())}
    , _markers{new Markers}
    , _options{new Options}
    , _timer{new InactivityTimer}
    , _screenRect{screenRect}
    , _view{view}
{
    _engine.rootContext()->setContextProperty("markers", _markers.get());
    _engine.rootContext()->setContextProperty("options", _options.get());
    _engine.rootContext()->setContextProperty("timer", _timer.get());
    _createDisplayGroupQmlItem(*parentWindow.rootObject());
    _displayGroupItem->setPosition(-screenRect.topLeft());
    _setBackground(_options->getBackgroundContent());
}

bool DisplayGroupRenderer::needRedraw() const
{
    return _options->getShowStatistics() || _options->getShowClock();
}

void DisplayGroupRenderer::setDisplayGroup(DisplayGroupPtr displayGroup)
{
    // Update the scene with the new information
    _engine.rootContext()->setContextProperty("displaygroup",
                                              displayGroup.get());

    // Update windows, creating new ones if needed
    QSet<QUuid> updatedWindows;
    const QQuickItem* parentItem = nullptr;
    const VisibilityHelper helper(*displayGroup, _screenRect);
    for (const auto& window : displayGroup->getContentWindows())
    {
        const auto& id = window->getID();

        updatedWindows.insert(id);

        if (!_windowItems.contains(id))
            _createWindowQmlItem(window);

        _windowItems[id]->update(window, helper.getVisibleArea(*window));

        // Update stacking order
        auto quickItem = _windowItems[id]->getQuickItem();
        if (parentItem)
            quickItem->stackAfter(parentItem);
        parentItem = quickItem;
    }

    // Remove old windows
    auto it = _windowItems.begin();
    while (it != _windowItems.end())
    {
        if (updatedWindows.contains(it.key()))
            ++it;
        else
            it = _windowItems.erase(it);
    }

    // Retain the new DisplayGroup
    _displayGroup = displayGroup;

    // Work around a bug in animation in Qt, where the opacity property
    // of the focus context may not always be restored to its original value.
    // See JIRA issue: DISCL-305
    if (!displayGroup->hasFocusedWindows() &&
        !displayGroup->hasFullscreenWindows() &&
        !displayGroup->hasVisiblePanels())
    {
        for (auto child : _displayGroupItem->childItems())
        {
            if (child->objectName() == "focuscontext")
                child->setProperty("opacity", 0.0);
        }
    }
}

void DisplayGroupRenderer::setMarkers(MarkersPtr markers)
{
    _engine.rootContext()->setContextProperty("markers", markers.get());
    _markers = markers; // Retain the new Markers
}

void DisplayGroupRenderer::setRenderingOptions(OptionsPtr options)
{
    _engine.rootContext()->setContextProperty("options", options.get());
    _setBackground(options->getBackgroundContent());
    _displayGroupItem->setVisible(!options->getShowTestPattern());
    _options = options; // Retain the new Options
}

void DisplayGroupRenderer::setTimer(InactivityTimerPtr timer)
{
    _engine.rootContext()->setContextProperty("timer", timer.get());
    _timer = timer; // Retain the new InactivityTimer
}

void DisplayGroupRenderer::updateRenderedFrames()
{
    const int frames = _displayGroupItem->property("frames").toInt();
    _displayGroupItem->setProperty("frames", frames + 1);
}

void DisplayGroupRenderer::_createDisplayGroupQmlItem(QQuickItem& parentItem)
{
    _engine.rootContext()->setContextProperty("displaygroup",
                                              _displayGroup.get());

    _displayGroupItem = qml::makeItem(_engine, QML_DISPLAYGROUP_URL);
    _displayGroupItem->setParentItem(&parentItem);
}

void DisplayGroupRenderer::_createWindowQmlItem(ContentWindowPtr window)
{
    const auto& id = window->getID();
    auto sync = _provider.createSynchronizer(*window, _view);
    _windowItems[id].reset(new QmlWindowRenderer(std::move(sync), window,
                                                 *_displayGroupItem,
                                                 _engine.rootContext()));
}

bool DisplayGroupRenderer::_hasBackgroundChanged(const QString& newUri) const
{
    ContentPtr prevContent = _options->getBackgroundContent();
    const QString& prevUri = prevContent ? prevContent->getURI() : QString();
    return newUri != prevUri;
}

void DisplayGroupRenderer::_setBackground(ContentPtr content)
{
    if (!content)
    {
        _backgroundWindowItem.reset();
        return;
    }

    if (!_hasBackgroundChanged(content->getURI()))
        return;

    auto window = boost::make_shared<ContentWindow>(content);
    window->setCoordinates(geometry::adjustAndCenter(*window, *_displayGroup));
    auto sync = _provider.createSynchronizer(*window, _view);
    _backgroundWindowItem.reset(
        new QmlWindowRenderer(std::move(sync), window, *_displayGroupItem,
                              _engine.rootContext(), true));

    DisplayGroup emptyGroup(_screenRect.size());
    const VisibilityHelper helper(emptyGroup, _screenRect);
    _backgroundWindowItem->update(window, helper.getVisibleArea(*window));
}
