/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
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
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#include "tools/LodTools.h"

#include <cassert>
#include <cmath>

LodTools::LodTools(const QSize& contentSize, const uint tileSize)
    : _contentSize(contentSize)
    , _tileSize(tileSize)
    , _maxLod(_computeMaxLod())
{
    assert(_tileSize > 0);
}

uint LodTools::getMaxLod() const
{
    return _maxLod;
}

QSize LodTools::getTilesArea(const uint lod) const
{
    return QSize(_contentSize.width() >> lod, _contentSize.height() >> lod);
}

QSize LodTools::getTilesCount(const uint lod) const
{
    const QSize size = getTilesArea(lod);
    return QSize(std::ceil((qreal)size.width() / _tileSize),
                 std::ceil((qreal)size.height() / _tileSize));
}

uint LodTools::getTilesCount() const
{
    uint count = 0;
    for (uint lod = 0; lod <= getMaxLod(); ++lod)
    {
        const QSize tiles = getTilesCount(lod);
        count += tiles.width() * tiles.height();
    }
    return count;
}

uint LodTools::getFirstTileId(const uint lod) const
{
    if (lod == getMaxLod())
        return 0;

    const QSize tiles = getTilesCount(lod + 1);
    const uint count = tiles.width() * tiles.height();
    return count + getFirstTileId(lod + 1);
}

LodTools::TileIndex LodTools::getTileIndex(const uint tileId) const
{
    uint lod = 0;
    uint firstTileId = getFirstTileId(lod);
    while (tileId < firstTileId)
        firstTileId = getFirstTileId(++lod);

    const int index = tileId - firstTileId;
    const QSize tilesCount = getTilesCount(lod);

    const uint x = index % tilesCount.width();
    const uint y = index / tilesCount.width();

    return TileIndex{x, y, lod};
}

QRect LodTools::getTileCoord(const uint tileId) const
{
    const auto index = getTileIndex(tileId);
    const QSize lodSize = getTilesArea(index.lod);

    if (index.lod == getMaxLod())
        return QRect(QPoint(0, 0), lodSize);

    const QSize tilesCount = getTilesCount(index.lod);
    const uint w = index.x < (uint)tilesCount.width()
                       ? _tileSize
                       : lodSize.width() % _tileSize;
    const uint h = index.y < (uint)tilesCount.height()
                       ? _tileSize
                       : lodSize.height() % _tileSize;

    return QRect(index.x * _tileSize, index.y * _tileSize, w, h);
}

const LodTools::TileInfos& LodTools::getAllTileInfos(const uint lod) const
{
    const QMutexLocker lock(&_lodTilesMapCacheMutex);
    if (!_lodTilesMapCache.count(lod))
    {
        LodTools::TileInfos& infos = _lodTilesMapCache[lod];

        const QSize tiles = getTilesCount(lod);

        uint id = getFirstTileId(lod);
        for (int y = 0; y < tiles.height(); ++y)
            for (int x = 0; x < tiles.width(); ++x, ++id)
                infos.push_back(TileInfo{id, getTileCoord(id)});
    }

    return _lodTilesMapCache[lod];
}

Indices LodTools::getVisibleTiles(const QRectF& area, const uint lod) const
{
    Indices indices;

    for (const auto& tile : getAllTileInfos(lod))
    {
        if (area.intersects(tile.coord))
            indices.insert(tile.id);
    }

    return indices;
}

uint LodTools::_computeMaxLod() const
{
    if (!_contentSize.isValid())
        return 0;

    uint maxLod = 0;
    uint maxDim = std::max(_contentSize.width(), _contentSize.height());
    while (maxDim > _tileSize)
    {
        maxDim = maxDim >> 1;
        ++maxLod;
    }
    return maxLod;
}
