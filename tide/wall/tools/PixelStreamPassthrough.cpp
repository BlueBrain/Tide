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

#include "PixelStreamPassthrough.h"

#include "data/StreamImage.h"

#include <deflect/server/Frame.h>
#include <deflect/server/TileDecoder.h>

PixelStreamPassthrough::PixelStreamPassthrough(deflect::server::FramePtr frame)
    : _frame(frame)
{
}

ImagePtr PixelStreamPassthrough::getTileImage(
    const uint tileIndex, deflect::server::TileDecoder& decoder)
{
    auto& tile = _frame->tiles.at(tileIndex);
    if (tile.format == deflect::Format::jpeg)
    {
#ifndef DEFLECT_USE_LEGACY_LIBJPEGTURBO
        decoder.decodeToYUV(tile);
#else
        decoder.decode(tile);
#endif
    }
    return std::make_shared<StreamImage>(_frame, tileIndex);
}

QRect PixelStreamPassthrough::getTileRect(const uint tileIndex) const
{
    return toRect(_frame->tiles.at(tileIndex));
}

Indices PixelStreamPassthrough::computeVisibleSet(const QRectF& visibleArea,
                                                  const uint channel) const
{
    Indices visibleSet;
    for (size_t i = 0; i < _frame->tiles.size(); ++i)
    {
        const auto& tile = _frame->tiles[i];
        if (tile.channel == channel && visibleArea.intersects(toRect(tile)))
            visibleSet.insert(i);
    }
    return visibleSet;
}
