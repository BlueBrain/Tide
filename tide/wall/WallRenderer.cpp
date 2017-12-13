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

#include "WallRenderer.h"

#include "CountdownStatus.h"
#include "DataProvider.h"
#include "DisplayGroupRenderer.h"
#include "ScreenLock.h"
#include "qmlUtils.h"
#include "scene/Background.h"
#include "scene/Markers.h"
#include "scene/Options.h"

#include <deflect/Frame.h>

#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>

namespace
{
const QUrl QML_ROOT_COMPONENT("qrc:/qml/wall/Root.qml");
}

WallRenderer::WallRenderer(WallRenderContext context, QQuickItem& parentItem)
    : _context{std::move(context)}
    , _qmlContext{*_context.engine.rootContext()}
    , _background{Background::create()}
    , _markers{Markers::create()}
    , _options{Options::create()}
    , _lock{ScreenLock::create()}
    , _countdownStatus{new CountdownStatus}
{
    _qmlContext.setContextProperty("markers", _markers.get());
    _qmlContext.setContextProperty("options", _options.get());
    _qmlContext.setContextProperty("countdownStatus", _countdownStatus.get());
    _qmlContext.setContextProperty("lock", _lock.get());

    _rootItem = qml::makeItem(_context.engine, QML_ROOT_COMPONENT);
    _rootItem->setParentItem(&parentItem);
    _rootItem->setSize(_context.wallSize);
    _rootItem->setPosition(-_context.screenRect.topLeft());

    _displayGroupRenderer =
        make_unique<DisplayGroupRenderer>(_context, *_rootItem);
}

WallRenderer::~WallRenderer()
{
}

void WallRenderer::setBackground(BackgroundPtr background)
{
    _setBackground(*background);
    _background = std::move(background);
}

bool WallRenderer::needRedraw() const
{
    return _options->getShowStatistics() || _options->getShowClock();
}

void WallRenderer::setDisplayGroup(DisplayGroupPtr displayGroup)
{
    _displayGroupRenderer->setDisplayGroup(displayGroup);
}

void WallRenderer::setMarkers(MarkersPtr markers)
{
    _qmlContext.setContextProperty("markers", markers.get());
    _markers = std::move(markers); // Retain the new Markers
}

void WallRenderer::setRenderingOptions(OptionsPtr options)
{
    _qmlContext.setContextProperty("options", options.get());
    _rootItem->setVisible(!options->getShowTestPattern());
    _options = std::move(options); // Retain the new Options
}

void WallRenderer::setScreenLock(ScreenLockPtr lock)
{
    _qmlContext.setContextProperty("lock", lock.get());
    _lock = std::move(lock); // Retain the new ScreenLock
}

void WallRenderer::setCountdownStatus(CountdownStatusPtr status)
{
    _qmlContext.setContextProperty("countdownStatus", status.get());
    _countdownStatus = std::move(status);
}

void WallRenderer::updateRenderedFrames()
{
    const int frames = _rootItem->property("frames").toInt();
    _rootItem->setProperty("frames", frames + 1);
}

void WallRenderer::_setBackground(const Background& background)
{
    const auto content = background.getContent();

    if (!content)
        _backgroundRenderer.reset();
    else if (_hasBackgroundChanged(content->getURI()))
        _backgroundRenderer =
            make_unique<BackgroundRenderer>(background, _context, *_rootItem);
}

bool WallRenderer::_hasBackgroundChanged(const QString& newUri) const
{
    return newUri != _background->getUri();
}
