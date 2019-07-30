/*********************************************************************/
/* Copyright (c) 2014-2018, EPFL/Blue Brain Project                  */
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

#include "FFMPEGMovie.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/pixdesc.h>
}

#include "FFMPEGFrame.h"
#include "FFMPEGPicture.h"
#include "FFMPEGUtils.h"
#include "FFMPEGVideoStream.h"
#include "utils/log.h"

#include <cmath>

#pragma clang diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

constexpr auto MIN_SEEK_DELTA_FRAMES = 5;

namespace
{
// Solve FFMPEG issue "insufficient thread locking around avcodec_open/close()"
int ffmpegLockManagerCallback(void** mutex, enum AVLockOp op)
{
    switch (op)
    {
    case AV_LOCK_CREATE:
        *mutex = static_cast<void*>(new std::mutex());
        return 0;
    case AV_LOCK_OBTAIN:
        static_cast<std::mutex*>(*mutex)->lock();
        return 0;
    case AV_LOCK_RELEASE:
        static_cast<std::mutex*>(*mutex)->unlock();
        return 0;
    case AV_LOCK_DESTROY:
        delete static_cast<std::mutex*>(*mutex);
        return 0;
    default:
        return 1;
    }
}

AVFormatContextPtr _createAvFormatContext(const QString& uri)
{
    // Read movie header information into _avFormatContext and allocate it
    AVFormatContext* avContext = nullptr;
    if (avformat_open_input(&avContext, uri.toLatin1(), 0, 0) != 0)
        throw std::runtime_error("error reading movie headers");
    auto avFormatContext = AVFormatContextPtr{avContext};

    // Read stream information into _avFormatContext->streams
    if (avformat_find_stream_info(avFormatContext.get(), NULL) < 0)
        throw std::runtime_error("error reading stream ");

#if LOG_THRESHOLD <= LOG_VERBOSE
    // print detail information about the input or output format
    av_dump_format(avFormatContext.get(), 0, uri.toLatin1(), 0);
#endif
    return avFormatContext;
}

struct FFMPEGStaticInit
{
    FFMPEGStaticInit()
    {
        av_lockmgr_register(&ffmpegLockManagerCallback);
        av_log_set_callback(avMessageLoger);
        av_log_set_level(AV_LOG_ERROR);
        av_register_all();
    }
};
static FFMPEGStaticInit instance;
} // namespace

FFMPEGMovie::FFMPEGMovie(const QString& uri)
    : _avFormatContext{_createAvFormatContext(uri)}
    , _videoStream{std::make_unique<FFMPEGVideoStream>(*_avFormatContext)}
{
    const auto format = _videoStream->getAVFormat();
    if (!FFMPEGUtils::isSupportedOutputFormat(format))
    {
        constexpr auto STR_LENGTH = 1000;
        char fmt[STR_LENGTH];
        av_get_pix_fmt_string(fmt, STR_LENGTH, format);

        print_log(LOG_WARN, LOG_AV,
                  "'%s': unsupported pixel format '%s'. Performance will be "
                  "non-optimal.",
                  uri.toStdString().c_str(), fmt);
    }
}

FFMPEGMovie::~FFMPEGMovie() = default;

unsigned int FFMPEGMovie::getWidth() const
{
    return isStereo() ? _videoStream->getWidth() / 2 : _videoStream->getWidth();
}

unsigned int FFMPEGMovie::getHeight() const
{
    return _videoStream->getHeight();
}

bool FFMPEGMovie::isStereo() const
{
    return _videoStream->isStereo();
}

double FFMPEGMovie::getPosition() const
{
    return _streamPosition;
}

double FFMPEGMovie::getDuration() const
{
    return _videoStream->getDuration();
}

double FFMPEGMovie::getFrameDuration() const
{
    return _videoStream->getFrameDuration();
}

PicturePtr FFMPEGMovie::getFrame(double posInSeconds)
{
    posInSeconds = std::max(0.0, std::min(posInSeconds, getDuration()));
    const auto frameDuration = _videoStream->getFrameDuration();
    const auto target = std::max(0.0, posInSeconds - frameDuration);
    const auto frameIndex = _videoStream->getFrameIndex(target);

    _frameIndex = frameIndex;
    _streamPosition = posInSeconds;
    int64_t frameIndexCurr = _frameLastDecode;
    _frameLastDecode = _frameIndex;

    // Seek back for loop or forward if too far away
    const auto streamDelta = frameIndex - frameIndexCurr;
    if (streamDelta < 0 || std::abs(streamDelta) > MIN_SEEK_DELTA_FRAMES)
    {
        if (!_videoStream->seekToNearestFullframe(frameIndex))
            return nullptr;
    }

    const auto targetTimestamp = _videoStream->getTimestamp(frameIndex);
    if (targetTimestamp == AV_NOPTS_VALUE)
        return nullptr;

    AVPacket packet;
    av_init_packet(&packet);

    PicturePtr picture;
    int avReadStatus = 0;

    auto frame = std::make_shared<FFMPEGFrame>();

    while ((avReadStatus = av_read_frame(_avFormatContext.get(), &packet)) >= 0)
    {
        bool success = _videoStream->decode(packet, *frame);
        const auto timestamp = frame->getTimestamp();

        // free the packet that was allocated by av_read_frame
        av_free_packet(&packet);

        if (success && timestamp >= targetTimestamp)
        {
            picture = std::make_shared<FFMPEGPicture>(
                FFMPEGUtils::convertToYUV(frame));
            break;
        }
    }

    // handle (rare) EOF case
    if (avReadStatus < 0)
        picture = PicturePtr();

    return picture;
}
