/*********************************************************************/
/* Copyright (c) 2019, EPFL/Blue Brain Project                       */
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

#include "FFMPEGUtils.h"

#include "FFMPEGFrame.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
#include <libswscale/swscale.h>
}

namespace FFMPEGUtils
{
bool isSupportedOutputFormat(const AVPixelFormat format)
{
    switch (format)
    {
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUVJ420P:
    case AV_PIX_FMT_YUV422P:
    case AV_PIX_FMT_YUVJ422P:
    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_YUVJ444P:
        return true;
    default:
        break;
    }

    return false;
}

TextureFormat determineOutputFormat(const AVPixelFormat format)
{
    switch (format)
    {
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUVJ420P:
        return TextureFormat::yuv420;
    case AV_PIX_FMT_YUV422P:
    case AV_PIX_FMT_YUVJ422P:
        return TextureFormat::yuv422;
    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_YUVJ444P:
        return TextureFormat::yuv444;
    default:
        throw std::logic_error("Unsupported pixel format '" +
                               std::to_string(static_cast<int>(format)) + "'");
    }
}

AVPixelFormat toAVPixelFormat(const TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::yuv420:
        return AV_PIX_FMT_YUV420P;
    case TextureFormat::yuv422:
        return AV_PIX_FMT_YUV422P;
    case TextureFormat::yuv444:
        return AV_PIX_FMT_YUV444P;
    default:
        throw std::logic_error("FFMPEGPicture: unsupported format");
    }
}

std::shared_ptr<FFMPEGFrame> convertToYUV(std::shared_ptr<FFMPEGFrame> frame)
{
    if (isSupportedOutputFormat(frame->getAVPixelFormat()))
        return frame;

    // Convert frame to yuv420
    constexpr auto DEST_FORMAT = AV_PIX_FMT_YUV420P;

    auto& avFrame = frame->getAVFrame();
    auto frameConv = std::make_shared<FFMPEGFrame>();
    auto& frameDest = frameConv->getAVFrame();

    av_image_alloc(frameDest.data, frameDest.linesize, avFrame.width,
                   avFrame.height, DEST_FORMAT, 1);

    frameDest.width = avFrame.width;
    frameDest.height = avFrame.height;
    frameDest.format = DEST_FORMAT;

    SwsContext* swsContext =
        sws_getContext(avFrame.width, avFrame.height, frame->getAVPixelFormat(),
                       frameDest.width, frameDest.height, DEST_FORMAT,
                       SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
    if (!swsContext)
        throw std::runtime_error("Could not create swscontext");

    sws_scale(swsContext, avFrame.data, avFrame.linesize, 0, avFrame.height,
              frameDest.data, frameDest.linesize);

    sws_freeContext(swsContext);

    frameConv->setDeallocateDataPointers();
    return frameConv;
}

} // namespace FFMPEGUtils
