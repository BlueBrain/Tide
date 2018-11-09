/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
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

#include "TiledSynchronizer.h"

#include "datasources/DataSource.h"
#include "qml/Tile.h"
#include "utils/stl.h"

TiledSynchronizer::TiledSynchronizer(const TileSwapPolicy policy)
    : _policy{policy}
{
}

void TiledSynchronizer::onSwapReady(TilePtr tile)
{
    if (_policy == SwapTilesSynchronously &&
        _syncSet.find(tile->getId()) != _syncSet.end())
    {
        _tilesReadyToSwap.insert(tile);
        _tilesReadySet.insert(tile->getId());
    }
    else
        tile->swapImage();
}

void TiledSynchronizer::updateTiles()
{
    if (!_tilesDirty)
        return;

    const auto visibleSet = _computeVisibleTilesAndAddMissingOnes();
    const auto removedTiles = set_difference(_visibleSet, visibleSet);

    if (_updateExistingTiles)
    {
        const auto currentTiles = set_difference(_visibleSet, removedTiles);
        _updateTiles(currentTiles);
    }

    if (_policy == SwapTilesSynchronously)
    {
        if (_updateExistingTiles)
        {
            _syncSet = visibleSet;
            _syncSwapPending = true;
        }
        else
            _syncSet = set_difference(_syncSet, removedTiles);
    }

    _removeTiles(removedTiles);

    _removeLaterSet = set_difference(_removeLaterSet, visibleSet);
    _visibleSet = visibleSet;

    _tilesDirty = false;
    _updateExistingTiles = false;
}

bool TiledSynchronizer::canSwapTiles() const
{
    return _syncSwapPending && set_difference(_syncSet, _tilesReadySet).empty();
}

void TiledSynchronizer::swapTiles()
{
    for (auto i : _removeLaterSet)
        emit removeTile(i);
    _removeLaterSet.clear();

    for (auto& tile : _tilesReadyToSwap)
        tile->swapImage();
    _tilesReadyToSwap.clear();
    _tilesReadySet.clear();
    _syncSet.clear();

    _syncSwapPending = false;
}

bool TiledSynchronizer::hasVisibleTiles() const
{
    return !_visibleSet.empty();
}

void TiledSynchronizer::markTilesDirty()
{
    _tilesDirty = true;
}

void TiledSynchronizer::markExistingTilesDirty()
{
    _updateExistingTiles = true;
}

Indices TiledSynchronizer::_computeVisibleTilesAndAddMissingOnes()
{
    Indices visibleSet;
    for (auto lod = getLod(); lod < getLodCount(); ++lod)
    {
        const auto visibleSetLod = _computeVisibleTiles(lod);
        const auto addedTilesLod = set_difference(visibleSetLod, _visibleSet);
        _addTiles(addedTilesLod, lod);

        visibleSet.insert(visibleSetLod.begin(), visibleSetLod.end());
    }
    return visibleSet;
}

Indices TiledSynchronizer::_computeVisibleTiles(const uint lod) const
{
    return getDataSource().computeVisibleSet(getVisibleTilesArea(lod), lod,
                                             getChannel());
}

void TiledSynchronizer::_addTiles(const Indices& tiles, const uint lod)
{
    const auto& source = getDataSource();
    const auto type = _getTextureType();
    const auto zOrder = getLodCount() - lod - 1;
    for (auto i : tiles)
        emit addTile(Tile::create(i, source.getTileRect(i), type), zOrder);
}

void TiledSynchronizer::_updateTiles(const Indices& tiles)
{
    for (auto i : tiles)
        emit updateTile(i, getDataSource().getTileRect(i));
}

void TiledSynchronizer::_removeTiles(const Indices& tiles)
{
    for (auto i : tiles)
        _removeTile(i);
}

void TiledSynchronizer::_removeTile(const size_t tileIndex)
{
    if (_policy == SwapTilesSynchronously && _syncSwapPending)
        _removeLaterSet.insert(tileIndex);
    else
        emit removeTile(tileIndex);
}

TextureType TiledSynchronizer::_getTextureType() const
{
    return getDataSource().isDynamic() ? TextureType::dynamic
                                       : TextureType::static_;
}
