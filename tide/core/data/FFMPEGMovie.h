/*********************************************************************/
/* Copyright (c) 2014-2017, EPFL/Blue Brain Project                  */
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

#ifndef FFMPEGMOVIE_H
#define FFMPEGMOVIE_H

#include "FFMPEGDefines.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/error.h>
    #include <libavutil/mathematics.h>
}

#include "types.h"

#include <QString>

/**
 * Read and play movies using the FFMPEG library.
 */
class FFMPEGMovie
{
public:
    /**
     * Constructor.
     * @param uri: the movie file to open.
     */
    FFMPEGMovie( const QString& uri );

    /** Destructor */
    ~FFMPEGMovie();

    /** Is the movie valid. */
    bool isValid() const;

    /** Get the frame width. */
    unsigned int getWidth() const;

    /** Get the frame height. */
    unsigned int getHeight() const;

    /** Get the current time position in seconds. */
    double getPosition() const;

    /** Get the movie duration in seconds. May be unavailable for some movies. */
    double getDuration() const;

    /** Get the duration of a frame in seconds. */
    double getFrameDuration() const;

    /** @return the format of the decoded movie frames. */
    TextureFormat getFormat() const;

    /** Set the format of the decoded movie frames, overwriting the default. */
    void setFormat( TextureFormat format );

    /**
     * Get a frame at the given position in seconds.
     *
     * @param posInSeconds request position in seconds; clamped if out-of-bounds
     * @return the decoded movie image that was closest to posInSeconds, nullptr
     *         otherwise
     */
    PicturePtr getFrame( double posInSeconds );

private:
    AVFormatContext* _avFormatContext = nullptr;
    std::unique_ptr<FFMPEGVideoStream> _videoStream;
    TextureFormat _format = TextureFormat::yuv420;

    double _streamPosition = 0.0;
    const bool _isValid = false;

    bool _open( const QString& uri );
    bool _createAvFormatContext( const QString& uri );
    void _releaseAvFormatContext();
    TextureFormat _determineOutputFormat( AVPixelFormat fileFormat,
                                          const QString& uri );
};

#endif
