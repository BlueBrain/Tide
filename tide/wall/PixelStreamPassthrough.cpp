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

#include "PixelStreamPassthrough.h"

#include "StreamImage.h"

#include <deflect/Frame.h>
#include <deflect/SegmentDecoder.h>

PixelStreamPassthrough::PixelStreamPassthrough(deflect::FramePtr frame)
    : _frame(frame)
{
}

ImagePtr PixelStreamPassthrough::getTileImage(const uint tileIndex,
                                              deflect::SegmentDecoder& decoder)
{
    auto& segment = _frame->segments.at(tileIndex);
    if (segment.parameters.dataType == deflect::DataType::jpeg)
    {
#ifndef DEFLECT_USE_LEGACY_LIBJPEGTURBO
        decoder.decodeToYUV(segment);
#else
        decoder.decode(segment);
#endif
    }
    return std::make_shared<StreamImage>(_frame, tileIndex);
}

QRect PixelStreamPassthrough::getTileRect(const uint tileIndex) const
{
    return toRect(_frame->segments.at(tileIndex).parameters);
}

TextureFormat PixelStreamPassthrough::getTileFormat(
    const uint tileIndex, deflect::SegmentDecoder& decoder) const
{
    return getFormat(_frame->segments.at(tileIndex), decoder);
}

Indices PixelStreamPassthrough::computeVisibleSet(
    const QRectF& visibleTilesArea) const
{
    Indices visibleSet;
    for (size_t i = 0; i < _frame->segments.size(); ++i)
    {
        const auto segmentRect = toRect(_frame->segments[i].parameters);
        if (visibleTilesArea.intersects(segmentRect))
            visibleSet.insert(i);
    }
    return visibleSet;
}
