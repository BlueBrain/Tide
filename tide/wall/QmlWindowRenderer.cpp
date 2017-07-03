/*********************************************************************/
/* Copyright (c) 2015-2017, EPFL/Blue Brain Project                  */
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

#include "QmlWindowRenderer.h"

#include "ContentSynchronizer.h"
#include "DataProvider.h"
#include "PixelStreamSynchronizer.h"
#include "Tile.h"
#include "qmlUtils.h"
#include "scene/ContentWindow.h"

#include <QQmlComponent>

namespace
{
const QUrl QML_WINDOW_URL("qrc:/qml/wall/WallContentWindow.qml");
const QString TILES_PARENT_OBJECT_NAME("TilesParent");
const QString ZOOM_CONTEXT_PARENT_OBJECT_NAME("ZoomContextParent");
}

QmlWindowRenderer::QmlWindowRenderer(
    std::unique_ptr<ContentSynchronizer> synchronizer,
    ContentWindowPtr contentWindow, QQuickItem& parentItem,
    QQmlContext* parentContext, const bool isBackground)
    : _synchronizer(std::move(synchronizer))
    , _contentWindow(contentWindow)
    , _windowContext(new QQmlContext(parentContext))
{
    connect(_synchronizer.get(), &ContentSynchronizer::addTile, this,
            &QmlWindowRenderer::_addTile);
    connect(_synchronizer.get(), &ContentSynchronizer::removeTile, this,
            &QmlWindowRenderer::_removeTile);
    connect(_synchronizer.get(), &ContentSynchronizer::updateTile, this,
            &QmlWindowRenderer::_updateTile);
    connect(_synchronizer.get(), &ContentSynchronizer::zoomContextTileChanged,
            this, &QmlWindowRenderer::_createZoomContextTile);

    _windowContext->setContextProperty("contentwindow", _contentWindow.get());
    _windowContext->setContextProperty("contentsync", _synchronizer.get());
    _windowItem.reset(qml::makeItem(*_windowContext->engine(), QML_WINDOW_URL,
                                    _windowContext.get()));
    _windowItem->setParentItem(&parentItem);
    _windowItem->setProperty("isBackground", isBackground);

    _createZoomContextTile();
}

QmlWindowRenderer::~QmlWindowRenderer()
{
    for (auto& tile : _tiles)
        tile.second->setParentItem(nullptr);
    _tiles.clear();

    _windowItem->setParentItem(nullptr);
    _windowItem.reset();
}

void QmlWindowRenderer::update(ContentWindowPtr contentWindow,
                               const QRectF& visibleArea)
{
    if (contentWindow->getVersion() != _contentWindow->getVersion())
    {
        _windowContext->setContextProperty("contentwindow",
                                           contentWindow.get());
        _contentWindow = contentWindow;
    }
    _synchronizer->update(*_contentWindow, visibleArea);
}

QQuickItem* QmlWindowRenderer::getQuickItem()
{
    return _windowItem.get();
}

void QmlWindowRenderer::_addTile(TilePtr tile)
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

void QmlWindowRenderer::_createZoomContextTile()
{
    if (_zoomContextTile)
    {
        _zoomContextTile->setParentItem(nullptr);
        _zoomContextTile.reset();
    }

    TilePtr tile = _synchronizer->getZoomContextTile();
    if (!tile)
        return;

    tile->setSizePolicy(Tile::FillParent);

    // Swap immediately, without going through the synchronizer
    connect(tile.get(), &Tile::readyToSwap, tile.get(), &Tile::swapImage);

    connect(tile.get(), &Tile::requestNextFrame, _synchronizer.get(),
            &ContentSynchronizer::onRequestNextFrame);

    _zoomContextTile = tile;

    auto item =
        _windowItem->findChild<QQuickItem*>(ZOOM_CONTEXT_PARENT_OBJECT_NAME);
    tile->setParentItem(item);
}

void QmlWindowRenderer::_removeTile(const uint tileIndex)
{
    if (!_tiles.count(tileIndex))
        return;

    _tiles[tileIndex]->disconnect(_synchronizer.get());
    _tiles[tileIndex]->setParentItem(nullptr);
    _tiles.erase(tileIndex);
}

void QmlWindowRenderer::_updateTile(const uint tileIndex,
                                    const QRect& coordinates,
                                    const TextureFormat format)
{
    if (_tiles.count(tileIndex))
        _tiles[tileIndex]->update(coordinates, format);
}
