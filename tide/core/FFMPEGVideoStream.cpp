/*********************************************************************/
/* Copyright (c) 2015, EPFL/Blue Brain Project                       */
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

#include "FFMPEGVideoStream.h"

#include "FFMPEGVideoFrameConverter.h"

#include "log.h"

#include <sstream>
#include <stdexcept>

#define USE_NEW_FFMPEG_API (LIBAVCODEC_VERSION_MAJOR >= 57)

FFMPEGVideoStream::FFMPEGVideoStream( AVFormatContext& avFormatContext )
    : _avFormatContext( avFormatContext )
    , _videoCodecContext( 0 ) // allocated if USE_NEW_FFMPEG_API, else shortcut
    , _videoStream( 0 )  // shortcut to _avFormatContext->streams[i]; don't free
    // Seeking parameters
    , _numFrames( 0 )
    , _frameDuration( 0.0 )
    , _frameDurationInSeconds( 0.0 )
{
    _findVideoStream();
    _openVideoStreamDecoder();
    _generateSeekingParameters();

    _frame.reset( new FFMPEGFrame );
    _frameConverter.reset( new FFMPEGVideoFrameConverter( *_videoCodecContext,
                                                          AV_PIX_FMT_RGBA ));
}

FFMPEGVideoStream::~FFMPEGVideoStream()
{
#if USE_NEW_FFMPEG_API
    avcodec_free_context( &_videoCodecContext );
#endif
}

PicturePtr FFMPEGVideoStream::decode( AVPacket& packet )
{
    if( !_decodeToAvFrame( packet ))
        return PicturePtr();

    return decodePictureForLastPacket();
}

int64_t FFMPEGVideoStream::decodeTimestamp( AVPacket& packet )
{
    if( !_decodeToAvFrame( packet ))
        return int64_t( -1 );

    return _frame->getTimestamp();
}

PicturePtr FFMPEGVideoStream::decodePictureForLastPacket()
{
    return _frameConverter->convert( *_frame );
}

bool FFMPEGVideoStream::_isVideoPacket( const AVPacket& packet ) const
{
    return packet.stream_index == _videoStream->index;
}

std::string _getAvError( const int errorCode )
{
    char errbuf[256];
    av_strerror( errorCode, errbuf, 256 );
    return std::string( errbuf );
}

bool FFMPEGVideoStream::_decodeToAvFrame( AVPacket& packet )
{
    if( !_isVideoPacket( packet ))
        return false;

#if USE_NEW_FFMPEG_API
    int errCode = avcodec_send_packet( _videoCodecContext, &packet );
    if( errCode < 0 )
    {
        put_flog( LOG_ERROR, "avcodec_send_packet returned error code '%i' : "
                  "'%s' in '%s'", errCode, _getAvError( errCode ).c_str(),
                  _avFormatContext.filename );
        return false;
    }

    errCode = avcodec_receive_frame( _videoCodecContext, &_frame->getAVFrame());
    if( errCode < 0 )
    {
        put_flog( LOG_ERROR, "avcodec_receive_frame returned error code '%i' : "
                  "'%s' in '%s'", errCode, _getAvError( errCode ).c_str(),
                  _avFormatContext.filename );
        return false;
    }
#else
    int frameDecodingComplete = 0;
    const int errCode = avcodec_decode_video2( _videoCodecContext,
                                               &_frame->getAVFrame(),
                                               &frameDecodingComplete,
                                               &packet );
    if( errCode < 0 )
    {
        put_flog( LOG_ERROR, "avcodec_decode_video2 returned error code '%i' "
                  "in '%s'", errCode, _avFormatContext.filename );
        return false;
    }

    // make sure we got a full video frame and convert the frame from its native
    // format to RGB
    if( !frameDecodingComplete )
    {
        put_flog( LOG_VERBOSE, "Frame could not be decoded entirely"
                               "(may be caused by seeking) in: '%s'",
                               _avFormatContext.filename );
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

double FFMPEGVideoStream::getDuration() const
{
    const double duration = (double)_videoStream->duration *
                            (double)_videoStream->time_base.num /
                            (double)_videoStream->time_base.den;
    return std::max( duration, 0.0 );
}

double FFMPEGVideoStream::getFrameDuration() const
{
    return _frameDurationInSeconds;
}

int64_t FFMPEGVideoStream::getFrameIndex( double timePositionInSec ) const
{
    const int64_t index = timePositionInSec / _frameDurationInSeconds;
    return index;
}

int64_t FFMPEGVideoStream::getTimestamp( double timePositionInSec ) const
{
    return getTimestamp( getFrameIndex( timePositionInSec ));
}

int64_t FFMPEGVideoStream::getTimestamp( int64_t frameIndex ) const
{
    if( frameIndex < 0 || ( _numFrames && frameIndex >= _numFrames ))
    {
        put_flog( LOG_WARN, "Invalid index: %i - valid range: [0, %i[ in: '%s'",
                  frameIndex, _numFrames, _avFormatContext.filename );
    }
    frameIndex = std::max( int64_t(0), std::min( frameIndex, _numFrames - 1 ));

    int64_t timestamp = frameIndex * _frameDuration;

    if( _videoStream->start_time != (int64_t)AV_NOPTS_VALUE )
        timestamp += _videoStream->start_time;

    return timestamp;
}

int64_t FFMPEGVideoStream::getFrameIndex( int64_t timestamp ) const
{
    if( _videoStream->start_time != (int64_t)AV_NOPTS_VALUE )
        timestamp -= _videoStream->start_time;

    const int64_t frameIndex = timestamp / _frameDuration;

    return frameIndex;
}

double FFMPEGVideoStream::getPositionInSec( const int64_t timestamp ) const
{
    return _frameDurationInSeconds * getFrameIndex( timestamp );
}

bool FFMPEGVideoStream::seekToNearestFullframe( int64_t frameIndex )
{
    if( frameIndex < 0 || ( _numFrames && frameIndex >= _numFrames ))
    {
        put_flog( LOG_WARN, "Invalid index: %i, range [0,%d[: '%s'",
                  frameIndex, _numFrames, _avFormatContext.filename );
    }

    frameIndex = std::max( int64_t(0), std::min( frameIndex, _numFrames - 1 ));

    const int64_t seek_target = getTimestamp( frameIndex );
    const int64_t seek_min = INT64_MIN;
    const int64_t seek_max = INT64_MAX;
    const int seek_flags = AVSEEK_FLAG_FRAME;

    if( avformat_seek_file( &_avFormatContext, _videoStream->index, seek_min,
                            seek_target, seek_max, seek_flags ) != 0 )
    {
        put_flog( LOG_ERROR, "seeking error, seeking aborted in: '%s'",
                  _avFormatContext.filename );
        return false;
    }

    avcodec_flush_buffers( _videoCodecContext );
    return true;
}

void FFMPEGVideoStream::_findVideoStream()
{
    for( unsigned int i = 0; i < _avFormatContext.nb_streams; ++i )
    {
        AVStream* stream = _avFormatContext.streams[i];
#if USE_NEW_FFMPEG_API
        if( stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO )
#else
        if( stream->codec->codec_type == AVMEDIA_TYPE_VIDEO )
#endif
        {
            _videoStream = stream; // Shortcut pointer - don't free
            return;
        }
    }

    throw std::runtime_error( "No video stream found in AVFormatContext" );
}

void FFMPEGVideoStream::_openVideoStreamDecoder()
{
    AVCodec* codec = nullptr;

#if USE_NEW_FFMPEG_API
    if( !( codec = avcodec_find_decoder( _videoStream->codecpar->codec_id )))
        throw std::runtime_error( "No decoder found for video stream" );

    _videoCodecContext = avcodec_alloc_context3( codec );
    if( !_videoCodecContext )
        throw std::runtime_error( "Could not allocate a decoding context" );

    const int error = avcodec_parameters_to_context( _videoCodecContext,
                                                     _videoStream->codecpar );
    if( error < 0 )
        throw std::runtime_error( "Could not init context from parameters" );
#else
    // Contains information about the codec that the stream is using
    _videoCodecContext = _videoStream->codec; // Shortcut - don't free

    codec = avcodec_find_decoder( _videoCodecContext->codec_id );
    if( !codec )
        throw std::runtime_error( "No decoder found for video stream" );
#endif

    // open codec
    const int ret = avcodec_open2( _videoCodecContext, codec, NULL );
    if( ret < 0 )
    {
        std::stringstream message;
        message << "Could not open codec, error code " << ret << ": " <<
                   _getAvError( ret );

        throw std::runtime_error( message.str( ));
    }
}

void FFMPEGVideoStream::_generateSeekingParameters()
{
    _numFrames = _videoStream->nb_frames;
    if( _numFrames == 0 )
    {
        const int den = _videoStream->avg_frame_rate.den * _videoStream->time_base.den;
        const int num = _videoStream->avg_frame_rate.num * _videoStream->time_base.num;
        if( den <= 0 || num <= 0 )
            throw std::runtime_error("cannot determine seeking paramters" );

        _numFrames = av_rescale( _videoStream->duration, num, den );
        if( _numFrames == 0 )
            throw std::runtime_error("cannot determine number of frames" );
    }

    _frameDuration = (double)_videoStream->duration / (double)_numFrames;

    const double timeBase = (double)_videoStream->time_base.num/
                            (double)_videoStream->time_base.den;
    _frameDurationInSeconds = _frameDuration * timeBase;

    put_flog( LOG_VERBOSE, "seeking parameters: start_time = %i,"
                           "duration_ = %i, numFrames_ = %i",
              _videoStream->start_time, _videoStream->duration, _numFrames );
    put_flog( LOG_VERBOSE, "frame_rate = %f, time_base = %f",
              1./_frameDurationInSeconds, timeBase );
    put_flog( LOG_VERBOSE, "frameDurationInSeconds_ = %f",
              _frameDurationInSeconds );
}
