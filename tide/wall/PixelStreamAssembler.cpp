/*********************************************************************/
/* Copyright (c) 2017-2018, EPFL/Blue Brain Project                  */
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

#include "PixelStreamAssembler.h"

#include "StreamImage.h"
#include "log.h"

#include <deflect/server/TileDecoder.h>

#include <cmath> //std::ceil

namespace
{
const uint32_t targetTileSize = 512;

bool _isValidSize(const uint32_t size)
{
    return size < targetTileSize && targetTileSize % size == 0;
}

bool _isValidSubtile(const deflect::server::Tile& tile)
{
    return _isValidSize(tile.width) && _isValidSize(tile.height);
}
}

PixelStreamAssembler::PixelStreamAssembler(deflect::server::FramePtr frame)
    : _frame{frame}
    , _frameSize{_frame->computeDimensions()}
{
    if (!_canAssemble())
        throw std::runtime_error("This frame cannot be assembled");

    _initTargetFrame();
}

ImagePtr PixelStreamAssembler::getTileImage(
    const uint tileIndex, deflect::server::TileDecoder& decoder)
{
    const auto sourceTiles = _findSourceTiles(tileIndex);

    _decodeSourceTiles(sourceTiles, decoder);
    _assembleTargetTile(tileIndex, sourceTiles);

    return std::make_shared<StreamImage>(_assembledFrame, tileIndex);
}

QRect PixelStreamAssembler::getTileRect(const uint tileIndex) const
{
    const uint tilesX = _getTilesX();
    const uint tilesY = _getTilesY();
    const uint x = tileIndex % tilesX;
    const uint y = tileIndex / tilesX;
    const uint paddingX = _frameSize.width() % targetTileSize;
    const uint paddingY = _frameSize.height() % targetTileSize;
    const uint w =
        (x < tilesX - 1) || paddingX == 0 ? targetTileSize : paddingX;
    const uint h =
        (y < tilesY - 1) || paddingY == 0 ? targetTileSize : paddingY;
    return QRect(x * targetTileSize, y * targetTileSize, w, h);
}

Indices PixelStreamAssembler::computeVisibleSet(
    const QRectF& visibleTilesArea) const
{
    Indices visibleSet;
    const auto tilesCount = _getTilesX() * _getTilesY();
    for (uint tileIndex = 0; tileIndex < tilesCount; ++tileIndex)
    {
        if (visibleTilesArea.intersects(getTileRect(tileIndex)))
            visibleSet.insert(tileIndex);
    }
    return visibleSet;
}

bool PixelStreamAssembler::_canAssemble() const
{
    const auto& sortedTiles = _frame->tiles;
    if (sortedTiles.size() <= 1)
        return false;

    const auto& firstTile = sortedTiles[0];

    if (!_isValidSubtile(firstTile))
        return false;

    const auto tileWidth = firstTile.width;
    const auto tileHeight = firstTile.height;

    uint currentX = 0;
    uint currentY = 0;
    bool eol = false;

    for (const auto& tile : sortedTiles)
    {
        if (tile.y != currentY)
        {
            if (!eol || tile.y != currentY + tileHeight)
                return false;
            // Next line
            currentY = tile.y;
            currentX = 0;
            eol = false;
        }

        if (tile.height != tileHeight)
        {
            if (currentY + tile.height != (uint)_frameSize.height())
                return false;
        }

        if (tile.x != currentX)
            return false;

        if (tile.width != tileWidth ||
            currentX + tileWidth >= (uint)_frameSize.width())
        {
            if (tile.x + tile.width != (uint)_frameSize.width())
                return false;

            eol = true;
        }

        currentX += tile.width;
    }
    return true;
}

uint PixelStreamAssembler::_getTilesX() const
{
    return std::ceil(float(_frameSize.width()) / targetTileSize);
}

uint PixelStreamAssembler::_getTilesY() const
{
    return std::ceil(float(_frameSize.height()) / targetTileSize);
}

void PixelStreamAssembler::_initTargetFrame()
{
    _assembledFrame.reset(new deflect::server::Frame);

    const auto tilesCount = _getTilesX() * _getTilesY();
    auto& tiles = _assembledFrame->tiles;
    tiles.resize(tilesCount);
    for (size_t i = 0; i < tilesCount; ++i)
    {
        const auto tileRect = getTileRect(i);
        tiles[i].width = tileRect.width();
        tiles[i].height = tileRect.height();
        tiles[i].x = tileRect.x();
        tiles[i].y = tileRect.y();
    }
}

Indices PixelStreamAssembler::_findSourceTiles(const uint tileIndex) const
{
    Indices indices;
    const auto tileRect = getTileRect(tileIndex);
    for (size_t i = 0; i < _frame->tiles.size(); ++i)
    {
        if (tileRect.intersects(toRect(_frame->tiles.at(i))))
            indices.insert(i);
    }
    return indices;
}

void PixelStreamAssembler::_decodeSourceTiles(
    const Indices& indices, deflect::server::TileDecoder& decoder)
{
    for (auto i : indices)
    {
        auto& tile = _frame->tiles.at(i);

        if (tile.format == deflect::Format::jpeg)
#ifndef DEFLECT_USE_LEGACY_LIBJPEGTURBO
            decoder.decodeToYUV(tile);
#else
            decoder.decode(tile);
#endif
    }
}

void PixelStreamAssembler::_assembleTargetTile(const uint tileIndex,
                                               const Indices& indices)
{
    auto& target = _assembledFrame->tiles[tileIndex];
    if (!target.imageData.isEmpty())
        return;

    const auto format = _frame->tiles[*indices.begin()].format;

    StreamImage image{_assembledFrame, tileIndex};
    target.format = format;
    const auto dataSize =
        image.getDataSize(0) + image.getDataSize(1) + image.getDataSize(2);
    target.imageData.resize(dataSize);
    for (auto i : indices)
    {
        const auto tile = StreamImage{_frame, (uint)i};
        image.copy(tile, tile.getPosition() - image.getPosition());
    }
}
