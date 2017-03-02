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

#ifndef FFMPEGVIDEOSTREAM_H
#define FFMPEGVIDEOSTREAM_H

#include "FFMPEGDefines.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/error.h>
    #include <libavutil/mathematics.h>
}

#include "types.h"

/** A video stream from an FFMPEG file. */
class FFMPEGVideoStream
{
public:
    /**
     * Constructor.
     * @param avFormatContext The FFMPEG context.
     * @throw std::runtime_error if an error occured during initialization
     */
    FFMPEGVideoStream( AVFormatContext& avFormatContext );

    /** Destructor. */
    ~FFMPEGVideoStream();

    /**
     * Decode a video packet.
     *
     * @param packet The av packet to decode
     * @param format The format for the decoded picture.
     * @return The decoded picture, or nullptr if the input is not a video
     *         packet or an error occured.
     */
    PicturePtr decode( AVPacket& packet, TextureFormat format );

    /**
     * Partially decode a video packet to determine its timestamp.
     *
     * This function is intended to help reach a desired timestamp, for instance
     * after seeking the file, without the need to decode the full picture.
     *
     * Only ONE of decode or decodeTimestamp must be called for a single packet,
     * otherwise the behaviour of the stream may become undefined (wrong picture
     * timestamp, for example).
     *
     * @sa getPictureForLastDecodedPacket()
     * @return the packet timestamp, or -1 on error.
     */
    int64_t decodeTimestamp( AVPacket& packet );

    /**
     * Call after a successful decodeTimestamp to get the corresponding picture.
     * @param format The format for the decoded picture.
     * @return The decoded picture, or nullptr if an error occured.
     */
    PicturePtr decodePictureForLastPacket( TextureFormat format );

    /** Get the width of the video stream. */
    unsigned int getWidth() const;

    /** Get the height of the video stream. */
    unsigned int getHeight() const;

    /**
     * Get the video stream duration in seconds.
     * May not always be available, in which case 0 is returned.
     */
    double getDuration() const;

    /** Get the duration of a frame in seconds. */
    double getFrameDuration() const;

    /** @return native format of the video stream. */
    AVPixelFormat getAVFormat() const;

    /** Get the frameIndex corresponding to the given time in seconds. */
    int64_t getFrameIndex( double timePositionInSec ) const;

    /** Get the timestamp corresponding to the given time in seconds. */
    int64_t getTimestamp( double timePositionInSec ) const;

    /** Get the timestamp corresponding to the given frameIndex. */
    int64_t getTimestamp( int64_t frameIndex ) const;

    /** Get the frameIndex corresponding to the given timestamp. */
    int64_t getFrameIndex( int64_t timestamp ) const ;

    /** Convert a timestamp to a time in seconds */
    double getPositionInSec( int64_t timestamp ) const;

    /** Seek to the nearest full frame in the video. */
    bool seekToNearestFullframe( int64_t frameIndex );

private:
    AVFormatContext& _avFormatContext;

    AVCodecContext* _videoCodecContext;
    AVStream* _videoStream;

    std::unique_ptr<FFMPEGFrame> _frame;
    std::unique_ptr<FFMPEGVideoFrameConverter> _frameConverter;

    // used for seeking
    int64_t _numFrames;
    double _frameDuration;
    double _frameDurationInSeconds;

    void _findVideoStream();
    void _openVideoStreamDecoder();
    void _generateSeekingParameters();

    bool _isVideoPacket( const AVPacket& packet ) const;
    bool _decodeToAvFrame( AVPacket& packet );
};

#endif
