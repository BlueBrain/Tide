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

#include "FFMPEGPicture.h"

#include "yuv.h"

#include <QOpenGLFunctions>

#pragma clang diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

FFMPEGPicture::FFMPEGPicture( const uint width, const uint height,
                              const TextureFormat format )
    : _width{ width }
    , _height{ height }
    , _format{ format }
    , _uvSize{ yuv::getUVSize( QSize( width, height ), format ) }
{
    switch( format )
    {
    case TextureFormat::rgba:
        _dataSize[0] = width * height * 4;
        _data[0].reset( new uint8_t[_dataSize[0]] );
        break;
    case TextureFormat::yuv420:
    case TextureFormat::yuv422:
    case TextureFormat::yuv444:
    {
        const auto uvDataSize = _uvSize.width() * _uvSize.height();
        _dataSize[0] = width * height;
        _dataSize[1] = uvDataSize;
        _dataSize[2] = uvDataSize;
        _data[0].reset( new uint8_t[_dataSize[0]] );
        _data[1].reset( new uint8_t[_dataSize[1]] );
        _data[2].reset( new uint8_t[_dataSize[2]] );
        break;
    }
    default:
        throw std::logic_error( "FFMPEGPicture: unsupported format" );
    }
}

int FFMPEGPicture::getWidth() const
{
    return _width;
}

int FFMPEGPicture::getHeight() const
{
    return _height;
}

QSize FFMPEGPicture::getSize( const uint texture ) const
{
    switch( texture )
    {
    case 0:
        return QSize{ getWidth(), getHeight() };
    case 1:
    case 2:
        return _uvSize;
    default:
        return QSize();
    }
}

const uint8_t* FFMPEGPicture::getData( const uint texture ) const
{
    if( texture >= _data.size( ))
        return nullptr;

    return _data[texture].get();
}

uint8_t* FFMPEGPicture::getData( const uint texture )
{
    if( texture >= _data.size( ))
        return nullptr;

    return _data[texture].get();
}

size_t FFMPEGPicture::getDataSize( const uint texture ) const
{
    if( texture >= _dataSize.size( ))
        return 0;

    return _dataSize[texture];
}

TextureFormat FFMPEGPicture::getFormat() const
{
    return _format;
}

uint FFMPEGPicture::getGLPixelFormat() const
{
    return getFormat() == TextureFormat::rgba ? GL_RGBA : GL_RED;
}

QImage FFMPEGPicture::toQImage() const
{
    if( getFormat() != TextureFormat::rgba )
        return QImage();

    return QImage( getData(), getWidth(), getHeight(),
                   QImage::Format_RGBA8888 );
}
