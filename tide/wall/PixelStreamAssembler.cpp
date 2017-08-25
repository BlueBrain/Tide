/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
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

#include "PixelStreamAssembler.h"

#include "StreamImage.h"
#include "log.h"

#include <deflect/SegmentDecoder.h>

#include <cmath> //std::ceil

namespace
{
const uint32_t targetTileSize = 512;

bool _isValidSize(const uint32_t size)
{
    return size < targetTileSize && targetTileSize % size == 0;
}

bool _isValidSubtile(const deflect::SegmentParameters& segment)
{
    return _isValidSize(segment.width) && _isValidSize(segment.height);
}
}

PixelStreamAssembler::PixelStreamAssembler(deflect::FramePtr frame)
    : _frame{frame}
    , _frameSize{_frame->computeDimensions()}
{
    if (!_canAssemble())
        throw std::runtime_error("This frame cannot be assembled");

    _initTargetFrame();
}

ImagePtr PixelStreamAssembler::getTileImage(const uint tileIndex,
                                            deflect::SegmentDecoder& decoder)
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
    const auto& sortedSegments = _frame->segments;
    if (sortedSegments.size() <= 1)
        return false;

    const auto& firstSegment = sortedSegments[0].parameters;

    if (!_isValidSubtile(firstSegment))
        return false;

    const auto tileWidth = firstSegment.width;
    const auto tileHeight = firstSegment.height;

    uint currentX = 0;
    uint currentY = 0;
    bool eol = false;

    for (const auto& segment : sortedSegments)
    {
        const auto& seg = segment.parameters;

        if (seg.y != currentY)
        {
            if (!eol || seg.y != currentY + tileHeight)
                return false;
            // Next line
            currentY = seg.y;
            currentX = 0;
            eol = false;
        }

        if (seg.height != tileHeight)
        {
            if (currentY + seg.height != (uint)_frameSize.height())
                return false;
        }

        if (seg.x != currentX)
            return false;

        if (seg.width != tileWidth ||
            currentX + tileWidth >= (uint)_frameSize.width())
        {
            if (seg.x + seg.width != (uint)_frameSize.width())
                return false;

            eol = true;
        }

        currentX += seg.width;
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
    _assembledFrame.reset(new deflect::Frame);

    const auto tilesCount = _getTilesX() * _getTilesY();
    auto& segments = _assembledFrame->segments;
    segments.resize(tilesCount);
    for (size_t i = 0; i < tilesCount; ++i)
    {
        const auto tileRect = getTileRect(i);
        segments[i].parameters.width = tileRect.width();
        segments[i].parameters.height = tileRect.height();
        segments[i].parameters.x = tileRect.x();
        segments[i].parameters.y = tileRect.y();
    }
}

Indices PixelStreamAssembler::_findSourceTiles(const uint tileIndex) const
{
    Indices indices;
    const auto tileRect = getTileRect(tileIndex);
    for (size_t i = 0; i < _frame->segments.size(); ++i)
    {
        const auto segmentRect = toRect(_frame->segments.at(i).parameters);
        if (tileRect.intersects(segmentRect))
            indices.insert(i);
    }
    return indices;
}

void PixelStreamAssembler::_decodeSourceTiles(const Indices& indices,
                                              deflect::SegmentDecoder& decoder)
{
    for (auto i : indices)
    {
        auto& segment = _frame->segments.at(i);

        if (segment.parameters.dataType == deflect::DataType::jpeg)
#ifndef DEFLECT_USE_LEGACY_LIBJPEGTURBO
            decoder.decodeToYUV(segment);
#else
            decoder.decode(segment);
#endif
    }
}

void PixelStreamAssembler::_assembleTargetTile(const uint tileIndex,
                                               const Indices& indices)
{
    auto& target = _assembledFrame->segments[tileIndex];
    if (!target.imageData.isEmpty())
        return;

    const auto type = _frame->segments[*indices.begin()].parameters.dataType;

    StreamImage image{_assembledFrame, tileIndex};
    target.parameters.dataType = type;
    const auto dataSize =
        image.getDataSize(0) + image.getDataSize(1) + image.getDataSize(2);
    target.imageData.resize(dataSize);
    for (auto i : indices)
    {
        const auto tile = StreamImage{_frame, (uint)i};
        image.copy(tile, tile.getPosition() - image.getPosition());
    }
}
