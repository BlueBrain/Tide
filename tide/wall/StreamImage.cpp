/*********************************************************************/
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project                  */
/*                          Daniel.Nachbaur@epfl.ch                  */
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

#include "StreamImage.h"

#include <deflect/Frame.h>

StreamImage::StreamImage(deflect::FramePtr frame, const uint tileIndex)
    : _frame{frame}
    , _tileIndex{tileIndex}
{
}

int StreamImage::getWidth() const
{
    return _frame->segments.at(_tileIndex).parameters.width;
}

int StreamImage::getHeight() const
{
    return _frame->segments.at(_tileIndex).parameters.height;
}

const uint8_t* StreamImage::getData(const uint texture) const
{
    const auto data = _frame->segments.at(_tileIndex).imageData.constData();
    if (getFormat() == TextureFormat::rgba || texture == 0)
        return reinterpret_cast<const uint8_t*>(data);

    size_t offset = getWidth() * getHeight();

    if (texture == 1)
        return reinterpret_cast<const uint8_t*>(data) + offset;

    if (texture == 2)
    {
        const auto uvSize = getTextureSize(1);
        offset += uvSize.width() * uvSize.height();
        return reinterpret_cast<const uint8_t*>(data) + offset;
    }
    return nullptr;
}

TextureFormat StreamImage::getFormat() const
{
    switch (_frame->segments.at(_tileIndex).parameters.dataType)
    {
    case deflect::DataType::rgba:
        return TextureFormat::rgba;
    case deflect::DataType::yuv444:
        return TextureFormat::yuv444;
    case deflect::DataType::yuv422:
        return TextureFormat::yuv422;
    case deflect::DataType::yuv420:
        return TextureFormat::yuv420;
    case deflect::DataType::jpeg:
    default:
        throw std::runtime_error("StreamImage texture is not decompressed");
    }
}

QPoint StreamImage::getPosition() const
{
    return QPoint(_frame->segments.at(_tileIndex).parameters.x,
                  _frame->segments.at(_tileIndex).parameters.y);
}

void StreamImage::copy(const StreamImage& image, const QPoint& position)
{
    const auto format = getFormat();
    if (image.getFormat() != format)
        throw std::runtime_error("Can't copy image with different format.");

    _copy(image, 0, position);
    if (format != TextureFormat::rgba)
    {
        const auto xShift = format == TextureFormat::yuv444 ? 0 : 1;
        const auto yShift = format == TextureFormat::yuv420 ? 1 : 0;
        const QPoint yuvPos{position.x() >> xShift, position.y() >> yShift};
        _copy(image, 1, yuvPos);
        _copy(image, 2, yuvPos);
    }
}

void StreamImage::_copy(const StreamImage& image, const uint texture,
                        const QPoint& position)
{
    const auto bpp = getFormat() == TextureFormat::rgba ? 4 : 1;

    auto src = image.getData(texture);
    const auto srcTexSize = image.getTextureSize(texture);
    const auto srcStride = srcTexSize.width() * bpp;

    auto dst = _getData(texture);
    const auto dstStride = getTextureSize(texture).width() * bpp;
    dst += position.x() * bpp + position.y() * dstStride;

    for (int line = 0; line < srcTexSize.height(); ++line)
    {
        assert(dst + srcStride <= _getData(texture) + getDataSize(texture));
        std::copy(src, src + srcStride, dst);
        src += srcStride;
        dst += dstStride;
        assert(src <= image.getData(texture) + image.getDataSize(texture));
    }
}

uint8_t* StreamImage::_getData(const uint texture)
{
    auto data = _frame->segments.at(_tileIndex).imageData.data();
    if (getFormat() == TextureFormat::rgba || texture == 0)
        return reinterpret_cast<uint8_t*>(data);

    size_t offset = getWidth() * getHeight();

    if (texture == 1)
        return reinterpret_cast<uint8_t*>(data) + offset;

    if (texture == 2)
    {
        const auto uvSize = getTextureSize(1);
        offset += uvSize.width() * uvSize.height();
        return reinterpret_cast<uint8_t*>(data) + offset;
    }
    return nullptr;
}
