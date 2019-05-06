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

#include "FFMPEGPicture.h"

#include "FFMPEGFrame.h"
#include "FFMPEGUtils.h"

#pragma clang diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
#include <libswscale/swscale.h>
}

constexpr auto MAX_CHANNELS = 3;

FFMPEGPicture::FFMPEGPicture(std::shared_ptr<FFMPEGFrame> frame)
    : _frame(frame)
{
    // NOTE: FFMpeg can and will often add padding at the end of every line.
    // Therefore we use the avframes' linesize as the width so that the
    // textures will be created with the right stride. Later, when displaying we
    // can then clip it by using a smaller view port.

    auto& avFrame = _frame->getAVFrame();
    _width = avFrame.linesize[0];
    _height = avFrame.height;

    const auto uvSize = getTextureSize(1);
    const int uvDataSize = uvSize.width() * uvSize.height();

    _dataSize[0] = _width * _height;
    _dataSize[1] = uvDataSize;
    _dataSize[2] = uvDataSize;
}

int FFMPEGPicture::getWidth() const
{
    return _width;
}

int FFMPEGPicture::getHeight() const
{
    return _height;
}

const uint8_t* FFMPEGPicture::getData(uint texture) const
{
    if (texture >= MAX_CHANNELS)
        return nullptr;

    return _frame->getAVFrame().data[texture];
}

TextureFormat FFMPEGPicture::getFormat() const
{
    return FFMPEGUtils::determineOutputFormat(_frame->getAVPixelFormat());
}

ColorSpace FFMPEGPicture::getColorSpace() const
{
    return ColorSpace::yCbCrVideo;
}

size_t FFMPEGPicture::getDataSize(uint texture) const
{
    if (texture >= MAX_CHANNELS)
        return 0;

    return _dataSize[texture];
}

QImage FFMPEGPicture::toQImage() const
{
    // NOTE: We use the viewport size to remove any potential line padding
    const auto viewPort = getViewPort();
    auto img =
        QImage(viewPort.width(), viewPort.height(), QImage::Format_RGBA8888);

    constexpr auto pixelSize = 4; // RGBA = 4 bytes
    constexpr auto destAvFormat = AV_PIX_FMT_RGBA;

    SwsContext* swsContext =
        sws_getContext(viewPort.width(), viewPort.height(),
                       _frame->getAVPixelFormat(), viewPort.width(),
                       viewPort.height(), destAvFormat, SWS_FAST_BILINEAR,
                       nullptr, nullptr, nullptr);
    if (!swsContext)
        return img;

    uint8_t* dstData = reinterpret_cast<uint8_t*>(img.bits());
    int linesize = viewPort.width() * pixelSize;

    auto& avFrame = _frame->getAVFrame();

    sws_scale(swsContext, avFrame.data, avFrame.linesize, 0, avFrame.height,
              (uint8_t* const*)&dstData, &linesize);

    sws_freeContext(swsContext);
    return img;
}

QRect FFMPEGPicture::getViewPort() const
{
    auto& avframe = _frame->getAVFrame();
    switch (_stereoView)
    {
    case StereoView::LEFT:
        return QRect(QPoint(0, 0), QSize(avframe.width / 2, avframe.height));
    case StereoView::RIGHT:
        return QRect(QPoint(avframe.width / 2, 0),
                     QSize(avframe.width / 2, avframe.height));
    case StereoView::NONE:
    default:
        return QRect(QPoint(0, 0), QSize(avframe.width, avframe.height));
    }
}

void FFMPEGPicture::setStereoView(const StereoView view)
{
    _stereoView = view;
}
