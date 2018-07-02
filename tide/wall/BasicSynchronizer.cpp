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

#include "BasicSynchronizer.h"

#include "DataSource.h"
#include "Tile.h"

BasicSynchronizer::BasicSynchronizer(std::shared_ptr<DataSource> source)
    : _dataSource(std::move(source))
{
    _dataSource->synchronizers.insert(this);

    connect(this, &ContentSynchronizer::zoomContextVisibleChanged,
            [this] { _zoomContextTileDirty = true; });
}

BasicSynchronizer::~BasicSynchronizer()
{
    _dataSource->synchronizers.erase(this);
}

void BasicSynchronizer::update(const Window& /*window*/,
                               const QRectF& visibleArea)
{
    if (!_tileAdded && !visibleArea.isEmpty())
        _addTile = true;
}

void BasicSynchronizer::updateTiles()
{
    if (_addTile)
        _createTile();

    if (_zoomContextTileDirty)
    {
        _zoomContextTileDirty = false;
        emit zoomContextTileChanged(getZoomContextVisible());
    }
}

bool BasicSynchronizer::canSwapTiles() const
{
    return false;
}

void BasicSynchronizer::swapTiles()
{
    // Swap not synchronized, done directly in onSwapReady()
}

QSize BasicSynchronizer::getTilesArea() const
{
    return getDataSource().getTilesArea(0, 0);
}

QString BasicSynchronizer::getStatistics() const
{
    QString stats;
    QTextStream stream(&stats);
    const auto area = getTilesArea();
    stream << "  res: " << area.width() << "x" << area.height();
    return stats;
}

void BasicSynchronizer::onSwapReady(TilePtr tile)
{
    tile->swapImage();
}

TilePtr BasicSynchronizer::createZoomContextTile() const
{
    return Tile::create(0, getDataSource().getTileRect(0));
}

void BasicSynchronizer::_createTile()
{
    if (_tileAdded)
        return;

    _tileAdded = true;
    _addTile = false;
    emit addTile(Tile::create(0, QRect(QPoint(), getTilesArea())));
    emit tilesAreaChanged();
}

const DataSource& BasicSynchronizer::getDataSource() const
{
    return *_dataSource;
}
