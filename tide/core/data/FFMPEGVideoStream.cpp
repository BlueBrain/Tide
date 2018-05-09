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

#include "FFMPEGVideoStream.h"

#include "FFMPEGFrame.h"
#include "FFMPEGVideoFrameConverter.h"
#include "log.h"

#include <sstream>
#include <stdexcept>

// FFMPEG 3.1
#define USE_NEW_FFMPEG_API (LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 48, 0))

// FFMPEG 2.3.x
#define HAS_STEREO_API (LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(55, 35, 100))

#if HAS_STEREO_API
extern "C" {
#include <libavutil/stereo3d.h>
}
#endif

FFMPEGVideoStream::FFMPEGVideoStream(AVFormatContext& avFormatContext)
    : _avFormatContext(avFormatContext)
    , _videoCodecContext(nullptr)
    , _videoStream(nullptr) // ptr to _avFormatContext->streams[i]; don't free
    // Seeking parameters
    , _numFrames(0)
    , _frameDuration(0.0)
    , _frameDurationInSeconds(0.0)
{
    _findVideoStream();
    _openVideoStreamDecoder();
    _generateSeekingParameters();

    _frame.reset(new FFMPEGFrame);
    _frameConverter.reset(new FFMPEGVideoFrameConverter);
}

FFMPEGVideoStream::~FFMPEGVideoStream()
{
#if USE_NEW_FFMPEG_API
    avcodec_free_context(&_videoCodecContext);
#else
    avcodec_close(_videoCodecContext);
#endif
}

PicturePtr FFMPEGVideoStream::decode(AVPacket& packet,
                                     const TextureFormat format)
{
    if (!_decodeToAvFrame(packet))
        return PicturePtr();

    return decodePictureForLastPacket(format);
}

int64_t FFMPEGVideoStream::decodeTimestamp(AVPacket& packet)
{
    if (!_decodeToAvFrame(packet))
        return AV_NOPTS_VALUE;

    return _frame->getTimestamp();
}

PicturePtr FFMPEGVideoStream::decodePictureForLastPacket(
    const TextureFormat format)
{
    return _frameConverter->convert(*_frame, format);
}

bool FFMPEGVideoStream::_isVideoPacket(const AVPacket& packet) const
{
    return packet.stream_index == _videoStream->index;
}

std::string _getAvError(const int errorCode)
{
    char errbuf[256];
    av_strerror(errorCode, errbuf, 256);
    return std::string(errbuf);
}

bool FFMPEGVideoStream::_decodeToAvFrame(AVPacket& packet)
{
    if (!_isVideoPacket(packet))
        return false;

#if USE_NEW_FFMPEG_API
    int errCode = avcodec_send_packet(_videoCodecContext, &packet);
    if (errCode < 0)
    {
        print_log(LOG_ERROR, LOG_AV,
                  "avcodec_send_packet returned error code '%i' : "
                  "'%s' in '%s'",
                  errCode, _getAvError(errCode).c_str(),
                  _avFormatContext.filename);
        return false;
    }

    errCode = avcodec_receive_frame(_videoCodecContext, &_frame->getAVFrame());
    if (errCode < 0)
    {
        print_log(LOG_ERROR, LOG_AV,
                  "avcodec_receive_frame returned error code '%i' : "
                  "'%s' in '%s'",
                  errCode, _getAvError(errCode).c_str(),
                  _avFormatContext.filename);
        return false;
    }
#else
    int frameDecodingComplete = 0;
    const int errCode =
        avcodec_decode_video2(_videoCodecContext, &_frame->getAVFrame(),
                              &frameDecodingComplete, &packet);
    if (errCode < 0)
    {
        print_log(LOG_ERROR, LOG_AV,
                  "avcodec_decode_video2 returned error code '%i' "
                  "in '%s'",
                  errCode, _avFormatContext.filename);
        return false;
    }

    // make sure we got a full video frame and convert the frame from its native
    // format to RGB
    if (!frameDecodingComplete)
    {
        print_log(LOG_VERBOSE, LOG_AV,
                  "Frame could not be decoded entirely"
                  "(may be caused by seeking) in: '%s'",
                  _avFormatContext.filename);
        return false;
    }
#endif
    return true;
}

unsigned int FFMPEGVideoStream::getWidth() const
{
    return _videoCodecContext->width;
}

unsigned int FFMPEGVideoStream::getHeight() const
{
    return _videoCodecContext->height;
}

bool FFMPEGVideoStream::isStereo() const
{
#if HAS_STEREO_API
    for (int i = 0; i < _videoStream->nb_side_data; ++i)
    {
        const auto& sideData = _videoStream->side_data[i];
        if (sideData.type == AV_PKT_DATA_STEREO3D)
            return ((AVStereo3D*)sideData.data)->type == AV_STEREO3D_SIDEBYSIDE;
    }
#endif
    return false;
}

double FFMPEGVideoStream::getDuration() const
{
    return std::max(_frameDurationInSeconds * _numFrames, 0.0);
}

double FFMPEGVideoStream::getFrameDuration() const
{
    return _frameDurationInSeconds;
}

AVPixelFormat FFMPEGVideoStream::getAVFormat() const
{
    return _videoCodecContext->pix_fmt;
}

int64_t FFMPEGVideoStream::getFrameIndex(const double timePositionInSec) const
{
    const int64_t index = timePositionInSec / _frameDurationInSeconds;
    return index;
}

int64_t FFMPEGVideoStream::getTimestamp(const double timePositionInSec) const
{
    return getTimestamp(getFrameIndex(timePositionInSec));
}

int64_t FFMPEGVideoStream::getTimestamp(int64_t frameIndex) const
{
    if (frameIndex < 0 || (_numFrames && frameIndex >= _numFrames))
    {
        print_log(LOG_WARN, LOG_AV,
                  "Invalid index: %i - valid range: [0, %i[ in: '%s'",
                  frameIndex, _numFrames, _avFormatContext.filename);
    }
    frameIndex = std::max(int64_t(0), std::min(frameIndex, _numFrames - 1));

    int64_t timestamp = frameIndex * _frameDuration;

    if (_videoStream->start_time != (int64_t)AV_NOPTS_VALUE)
        timestamp += _videoStream->start_time;

    return timestamp;
}

int64_t FFMPEGVideoStream::getFrameIndex(int64_t timestamp) const
{
    if (_videoStream->start_time != (int64_t)AV_NOPTS_VALUE)
        timestamp -= _videoStream->start_time;

    const int64_t frameIndex = timestamp / _frameDuration;

    return frameIndex;
}

double FFMPEGVideoStream::getPositionInSec(const int64_t timestamp) const
{
    return _frameDurationInSeconds * getFrameIndex(timestamp);
}

bool FFMPEGVideoStream::seekToNearestFullframe(int64_t frameIndex)
{
    if (frameIndex < 0 || (_numFrames && frameIndex >= _numFrames))
    {
        print_log(LOG_WARN, LOG_AV, "Invalid index: %i, range [0,%d[: '%s'",
                  frameIndex, _numFrames, _avFormatContext.filename);
    }

    frameIndex = std::max(int64_t(0), std::min(frameIndex, _numFrames - 1));

    const int64_t seek_target = getTimestamp(frameIndex);
    const int64_t seek_min = INT64_MIN;
    const int64_t seek_max = INT64_MAX;
    const int seek_flags = AVSEEK_FLAG_FRAME;

    if (avformat_seek_file(&_avFormatContext, _videoStream->index, seek_min,
                           seek_target, seek_max, seek_flags) != 0)
    {
        print_log(LOG_ERROR, LOG_AV, "seeking error, seeking aborted in: '%s'",
                  _avFormatContext.filename);
        return false;
    }

    avcodec_flush_buffers(_videoCodecContext);
    return true;
}

void FFMPEGVideoStream::_findVideoStream()
{
    for (unsigned int i = 0; i < _avFormatContext.nb_streams; ++i)
    {
        AVStream* stream = _avFormatContext.streams[i];
#if USE_NEW_FFMPEG_API
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
#else
        if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
#endif
        {
            _videoStream = stream; // Shortcut pointer - don't free
            return;
        }
    }

    throw std::runtime_error("No video stream found in AVFormatContext");
}

void FFMPEGVideoStream::_openVideoStreamDecoder()
{
    AVCodec* codec = nullptr;

#if USE_NEW_FFMPEG_API
    if (!(codec = avcodec_find_decoder(_videoStream->codecpar->codec_id)))
        throw std::runtime_error("No decoder found for video stream");

    _videoCodecContext = avcodec_alloc_context3(codec);
    if (!_videoCodecContext)
        throw std::runtime_error("Could not allocate a decoding context");

    const int error = avcodec_parameters_to_context(_videoCodecContext,
                                                    _videoStream->codecpar);
    if (error < 0)
        throw std::runtime_error("Could not init context from parameters");
#else
    if (!(codec = avcodec_find_decoder(_videoStream->codec->codec_id)))
        throw std::runtime_error("No decoder found for video stream");

    _videoCodecContext = _videoStream->codec; // ptr, allocated by avcodec_open2
#endif

    const int ret = avcodec_open2(_videoCodecContext, codec, NULL);
    if (ret < 0)
    {
        std::stringstream message;
        message << "Could not open codec, error code " << ret << ": "
                << _getAvError(ret);
        throw std::runtime_error(message.str());
    }

    if (_videoCodecContext->pix_fmt == AV_PIX_FMT_NONE)
        throw std::runtime_error("video stream has undefined pixel format");
}

void FFMPEGVideoStream::_generateSeekingParameters()
{
    int64_t duration = _videoStream->duration;
    const auto& timeBase = _videoStream->time_base;

    // Webm files do not have per-stream duration information, only global. see:
    // http://stackoverflow.com/questions/32532122/
    // finding-duration-number-of-frames-of-webm-using-ffmpeg-libavformat
    if (duration <= 0)
    {
        // Transform AVFormatContext duration which is in AV_TIME_BASE units
        // (microseconds) into _videoStream->time_base units.
        // The resulting _frameDuration has to be in video stream units to
        // provide correct timestamps for avformat_seek_file() for instance.
        const auto num = int64_t(timeBase.den);
        const auto den = int64_t(timeBase.num) * AV_TIME_BASE;
        duration = av_rescale(_avFormatContext.duration, num, den);
    }

    // Estimate number of frames if unavailable
    _numFrames = _videoStream->nb_frames;
    if (_numFrames == 0)
    {
        const auto& frameRate = _videoStream->avg_frame_rate;
        const auto num = int64_t(frameRate.num) * int64_t(timeBase.num);
        const auto den = int64_t(frameRate.den) * int64_t(timeBase.den);
        if (num <= 0 || den <= 0)
            throw std::runtime_error("cannot determine seeking paramters");
        _numFrames = av_rescale(duration, num, den);
    }
    if (_numFrames <= 0)
        throw std::runtime_error("cannot determine number of frames");

    _frameDuration = double(duration) / double(_numFrames);

    const auto frameDurationUnits = double(timeBase.num) / double(timeBase.den);
    _frameDurationInSeconds = _frameDuration * frameDurationUnits;

    print_log(LOG_VERBOSE, LOG_AV,
              "seeking parameters: start_time = %i,"
              "duration = %i, numFrames = %i",
              _videoStream->start_time, duration, _numFrames);
    print_log(LOG_VERBOSE, LOG_AV, "frame_rate = %f, time_base = %f",
              1. / _frameDurationInSeconds, timeBase);
    print_log(LOG_VERBOSE, LOG_AV, "frameDurationInSeconds = %f",
              _frameDurationInSeconds);
}
