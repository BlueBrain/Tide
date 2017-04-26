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

#ifndef LODTOOLS_H
#define LODTOOLS_H

#include "types.h"

#include <QMutex>
#include <map>

/**
 * Tools to compute LOD pyramid data for a 2D tiled image.
 */
class LodTools
{
public:
    struct TileIndex
    {
        uint x;
        uint y;
        uint lod;
    };

    struct TileInfo
    {
        uint id;
        QRect coord;
    };
    typedef std::vector<TileInfo> TileInfos;

    /**
     * Constructor
     * @param contentSize the size of the full resolution content
     * @param tileSize the size of the tiles to subdivide the content
     */
    LodTools(const QSize& contentSize, uint tileSize);

    /** @return the max LOD level (top of pyramid, lowest resolution) */
    uint getMaxLod() const;

    /** @return the area covered by the tiles at the given lod. */
    QSize getTilesArea(uint lod) const;

    /** @return the number of tiles for the given lod. */
    QSize getTilesCount(uint lod) const;

    /** @return the total number of tiles in the pyramid. */
    uint getTilesCount() const;

    /** @return the index of the first tile of the given lod. */
    uint getFirstTileId(uint lod) const;

    /** @return the index of the given tile. */
    TileIndex getTileIndex(uint tileId) const;

    /** @return the coordinates of the given tile. */
    QRect getTileCoord(uint tileId) const;

    /** @return all the tile coordinates for the given lod. */
    const TileInfos& getAllTileInfos(uint lod) const;

    /** @return the IDs of the tiles of the given LOD visible in the area. */
    Indices getVisibleTiles(const QRectF& area, uint lod) const;

private:
    const QSize _contentSize;
    const uint _tileSize;
    const uint _maxLod;

    mutable QMutex _lodTilesMapCacheMutex;
    typedef std::map<size_t, TileInfos> LodTilesMap;
    mutable LodTilesMap _lodTilesMapCache;

    uint _computeMaxLod() const;
};

#endif
