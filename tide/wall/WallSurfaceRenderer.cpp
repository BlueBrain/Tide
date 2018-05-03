/*********************************************************************/
/* Copyright (c) 2014-2018, EPFL/Blue Brain Project                  */
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

#include "WallSurfaceRenderer.h"

#include "DataProvider.h"
#include "DisplayGroupRenderer.h"
#include "qmlUtils.h"
#include "scene/Background.h"
#include "scene/CountdownStatus.h"
#include "scene/DisplayGroup.h"
#include "scene/Markers.h"
#include "scene/Options.h"
#include "scene/ScreenLock.h"

#include <deflect/server/Frame.h>

#include <QQmlContext>
#include <QQuickItem>

namespace
{
const QUrl QML_CONTROL_SURFACE_URL("qrc:/qml/wall/WallControlSurface.qml");
const QUrl QML_BASIC_SURFACE_URL("qrc:/qml/wall/WallBasicSurface.qml");
}

WallSurfaceRenderer::WallSurfaceRenderer(WallRenderContext context,
                                         QQuickItem& parentItem)
    : _context{std::move(context)}
    , _qmlContext{*_context.engine.rootContext()}
    , _background{Background::create()}
    , _countdownStatus{new CountdownStatus}
    , _displayGroup{new DisplayGroup{QSize(1, 1)}}
    , _markers{Markers::create(_context.surfaceIndex)}
    , _options{Options::create()}
    , _screenLock{ScreenLock::create()}
{
    _setContextProperties();
    _createSurfaceItem(parentItem);
    _createGroupRenderer();
}

WallSurfaceRenderer::~WallSurfaceRenderer()
{
}

void WallSurfaceRenderer::setBackground(BackgroundPtr background)
{
    _setBackground(*background);
    _background = std::move(background);
}

bool WallSurfaceRenderer::needRedraw() const
{
    return _options->getShowStatistics() || _options->getShowClock();
}

void WallSurfaceRenderer::setDisplayGroup(DisplayGroupPtr displayGroup)
{
    _qmlContext.setContextProperty("displaygroup", displayGroup.get());
    _displayGroup = displayGroup;

    _displayGroupRenderer->setDisplayGroup(displayGroup);
}

void WallSurfaceRenderer::setMarkers(MarkersPtr markers)
{
    if (markers->getSurfaceIndex() != _context.surfaceIndex)
        return;

    _qmlContext.setContextProperty("markers", markers.get());
    _markers = std::move(markers); // Retain the new Markers
}

void WallSurfaceRenderer::setRenderingOptions(OptionsPtr options)
{
    _qmlContext.setContextProperty("options", options.get());
    _surfaceItem->setVisible(!options->getShowTestPattern());
    _options = std::move(options); // Retain the new Options
}

void WallSurfaceRenderer::setScreenLock(ScreenLockPtr lock)
{
    _qmlContext.setContextProperty("lock", lock.get());
    _screenLock = std::move(lock); // Retain the new ScreenLock
}

void WallSurfaceRenderer::setCountdownStatus(CountdownStatusPtr status)
{
    _qmlContext.setContextProperty("countdownStatus", status.get());
    _countdownStatus = std::move(status);
}

void WallSurfaceRenderer::updateRenderedFrames()
{
    const int frames = _surfaceItem->property("frames").toInt();
    _surfaceItem->setProperty("frames", frames + 1);
}

void WallSurfaceRenderer::_setContextProperties()
{
    _qmlContext.setContextProperty("countdownStatus", _countdownStatus.get());
    _qmlContext.setContextProperty("displaygroup", _displayGroup.get());
    _qmlContext.setContextProperty("markers", _markers.get());
    _qmlContext.setContextProperty("options", _options.get());
    _qmlContext.setContextProperty("lock", _screenLock.get());
}

void WallSurfaceRenderer::_createSurfaceItem(QQuickItem& parentItem)
{
    if (_context.surfaceIndex == 0)
        _surfaceItem = qml::makeItem(_context.engine, QML_CONTROL_SURFACE_URL);
    else
        _surfaceItem = qml::makeItem(_context.engine, QML_BASIC_SURFACE_URL);

    _surfaceItem->setSize(_context.wallSize);
    _surfaceItem->setPosition(-_context.screenRect.topLeft());
    _surfaceItem->setParentItem(&parentItem);
}

void WallSurfaceRenderer::_createGroupRenderer()
{
    _displayGroupRenderer =
        std::make_unique<DisplayGroupRenderer>(_context, *_surfaceItem);
}

void WallSurfaceRenderer::_setBackground(const Background& background)
{
    const auto content = background.getContent();

    if (!content)
        _backgroundRenderer.reset();
    else if (_hasBackgroundChanged(content->getURI()))
        _backgroundRenderer =
            std::make_unique<BackgroundRenderer>(background, _context,
                                                 *_surfaceItem);
}

bool WallSurfaceRenderer::_hasBackgroundChanged(const QString& newUri) const
{
    return newUri != _background->getUri();
}
