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

namespace
{
Indices _mapToGlobalIndices(const Indices& perChannelIndices,
                            const size_t channelOffset)
{
    Indices globalIndices;
    for (auto index : perChannelIndices)
        globalIndices.insert(index + channelOffset);
    return globalIndices;
}
}

PixelStreamAssembler::PixelStreamAssembler(deflect::server::FramePtr frame)
{
    if (!_parseChannels(frame))
        throw std::runtime_error("This frame cannot be assembled");
}

ImagePtr PixelStreamAssembler::getTileImage(
    uint tileIndex, deflect::server::TileDecoder& decoder)
{
    const auto& channel = _getChannel(tileIndex);
    return channel.assembler.getTileImage(tileIndex - channel.offset, decoder);
}

QRect PixelStreamAssembler::getTileRect(const uint tileIndex) const
{
    const auto& channel = _getChannel(tileIndex);
    return channel.assembler.getTileRect(tileIndex - channel.offset);
}

Indices PixelStreamAssembler::computeVisibleSet(const QRectF& visibleArea,
                                                const uint channelIndex) const
{
    const auto& channel = _channels.at(channelIndex);
    const auto indices =
        channel.assembler.computeVisibleSet(visibleArea, channelIndex);
    return _mapToGlobalIndices(indices, channel.offset);
}

bool PixelStreamAssembler::_parseChannels(deflect::server::FramePtr frame)
{
    if (frame->tiles.empty())
        return false;

    size_t tileIndexOffset = 0;
    for (const auto& tile : frame->tiles)
    {
        const auto channelIndex = tile.channel;
        if (channelIndex == _channels.size())
        {
            _channels.emplace_back(frame, channelIndex, tileIndexOffset);
            tileIndexOffset += _channels.back().tilesCount;
        }
        else if (channelIndex != _channels.size() - 1)
            return false;
    }
    return true;
}

const PixelStreamAssembler::Channel& PixelStreamAssembler::_getChannel(
    const uint tileIndex) const
{
    for (const auto& channel : _channels)
        if (tileIndex < channel.offset + channel.tilesCount)
            return channel;

    throw std::out_of_range("tileIndex is out of range");
}
