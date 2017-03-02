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

#ifndef FFMPEGPICTURE_H
#define FFMPEGPICTURE_H

#include "Image.h"

#include <QByteArray>
#include <QImage>

#include <array>

/**
 * A decoded frame of the movie stream in RGBA or YUV format.
 */
class FFMPEGPicture : public Image
{
public:
    /** Allocate a new picture. */
    FFMPEGPicture( uint width, uint height, TextureFormat format );

    /** @copydoc Image::getWidth */
    int getWidth() const final;

    /** @copydoc Image::getHeight */
    int getHeight() const final;

    /** @copydoc Image::getData */
    const uint8_t* getData( uint texture = 0 ) const final;

    /** @return write access to fill a given image texture plane. */
    uint8_t* getData( uint texture );

    /** @return data size of a given image texture plane. */
    size_t getDataSize( uint texture ) const;

    /** @copydoc Image::getTextureSize */
    QSize getTextureSize( uint texture = 0 ) const final;

    /** @copydoc Image::getFormat */
    TextureFormat getFormat() const final;

    /** @copydoc Image::getGLPixelFormat */
    uint getGLPixelFormat() const final;

    /** @return the picture as a QImage, or an empty one if format != rgba. */
    QImage toQImage() const;

private:
    const uint _width;
    const uint _height;
    const TextureFormat _format;
    const QSize _uvSize;
    std::array<QByteArray, 3> _data;
};

#endif
