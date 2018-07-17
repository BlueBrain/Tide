/*********************************************************************/
/* Copyright (c) 2015-2018, EPFL/Blue Brain Project                  */
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

#include "WindowRenderer.h"

#include "ContentSynchronizer.h"
#include "DataProvider.h"
#include "PixelStreamSynchronizer.h"
#include "Tile.h"
#include "scene/Window.h"
#include "utils/qml.h"

namespace
{
const QUrl QML_WINDOW_URL("qrc:/qml/wall/WallWindow.qml");
const QString TILES_PARENT_OBJECT_NAME("TilesParent");
const QString ZOOM_CONTEXT_PARENT_OBJECT_NAME("ZoomContextParent");
}

WindowRenderer::WindowRenderer(
    std::unique_ptr<ContentSynchronizer> synchronizer, WindowPtr window,
    QQuickItem& parentItem, QQmlContext* parentContext, const bool isBackground)
    : _synchronizer(std::move(synchronizer))
    , _window(window)
    , _windowContext(new QQmlContext(parentContext))
{
    connect(_synchronizer.get(), &ContentSynchronizer::addTile, this,
            &WindowRenderer::_addTile);
    connect(_synchronizer.get(), &ContentSynchronizer::removeTile, this,
            &WindowRenderer::_removeTile);
    connect(_synchronizer.get(), &ContentSynchronizer::updateTile, this,
            &WindowRenderer::_updateTile);
    connect(_synchronizer.get(), &ContentSynchronizer::zoomContextTileChanged,
            this, &WindowRenderer::_updateZoomContextTile);

    _windowContext->setContextProperty("window", _window.get());
    _windowContext->setContextProperty("contentsync", _synchronizer.get());
    _windowItem = qml::makeItem(*_windowContext->engine(), QML_WINDOW_URL,
                                _windowContext.get());
    _windowItem->setParentItem(&parentItem);
    _windowItem->setProperty("isBackground", isBackground);
}

WindowRenderer::~WindowRenderer()
{
    if (_zoomContextTile)
        _removeZoomContextTile();

    for (auto& tile : _tiles)
        tile.second->setParentItem(nullptr);
    _tiles.clear();
}

void WindowRenderer::update(WindowPtr window, const QRectF& visibleArea)
{
    if (window->getVersion() != _window->getVersion())
    {
        _windowContext->setContextProperty("window", window.get());
        _window = window;
    }
    _synchronizer->update(*_window, visibleArea);
}

QQuickItem* WindowRenderer::getQuickItem()
{
    return _windowItem.get();
}

void WindowRenderer::_addTile(TilePtr tile)
{
    connect(tile.get(), &Tile::readyToSwap, _synchronizer.get(),
            &ContentSynchronizer::onSwapReady);

    connect(tile.get(), &Tile::requestNextFrame, _synchronizer.get(),
            &ContentSynchronizer::onRequestNextFrame);

    _tiles[tile->getId()] = tile;

    auto item = _windowItem->findChild<QQuickItem*>(TILES_PARENT_OBJECT_NAME);
    tile->setParentItem(item);

    connect(item, SIGNAL(showTilesBordersValueChanged(bool)), tile.get(),
            SLOT(setShowBorder(bool)));
    tile->setShowBorder(item->property("showTilesBorder").toBool());

    tile->requestNextFrame(tile);
}

QQuickItem* WindowRenderer::_getZoomContextParentItem() const
{
    return _windowItem->findChild<QQuickItem*>(ZOOM_CONTEXT_PARENT_OBJECT_NAME);
}

void WindowRenderer::_updateZoomContextTile(const bool visible)
{
    if (_zoomContextTile)
        _removeZoomContextTile();

    if (visible)
        _addZoomContextTile();
}

void WindowRenderer::_addZoomContextTile()
{
    auto tile = _synchronizer->createZoomContextTile();
    if (!tile)
        return;

    tile->setSizePolicy(Tile::FillParent);

    // Swap immediately, without going through the synchronizer
    connect(tile.get(), &Tile::readyToSwap, tile.get(), &Tile::swapImage);

    connect(tile.get(), &Tile::requestNextFrame, _synchronizer.get(),
            &ContentSynchronizer::onRequestNextFrame);

    _zoomContextTile = tile;

    tile->setParentItem(_getZoomContextParentItem());
    tile->requestNextFrame(tile);
}

void WindowRenderer::_removeZoomContextTile()
{
    _zoomContextTile->setParentItem(nullptr);
    _zoomContextTile.reset();
}

void WindowRenderer::_removeTile(const uint tileIndex)
{
    auto tileIt = _tiles.find(tileIndex);
    if (tileIt == _tiles.end())
        return;

    auto& tile = tileIt->second;
    tile->disconnect(_synchronizer.get());
    tile->setParentItem(nullptr);
    _tiles.erase(tileIt);
}

void WindowRenderer::_updateTile(const uint tileIndex, const QRect& coordinates)
{
    auto tileIt = _tiles.find(tileIndex);
    if (tileIt != _tiles.end())
        tileIt->second->update(coordinates);
}
