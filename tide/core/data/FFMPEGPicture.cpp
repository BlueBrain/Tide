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

#pragma clang diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

FFMPEGPicture::FFMPEGPicture( const uint width, const uint height,
                              const TextureFormat format )
    : _width{ width }
    , _height{ height }
    , _format{ format }
{
    switch( format )
    {
    case TextureFormat::rgba:
        _data[0] = QByteArray{ int(width * height * 4), Qt::Uninitialized };
        break;
    case TextureFormat::yuv420:
    case TextureFormat::yuv422:
    case TextureFormat::yuv444:
    {
        const auto uvSize = getTextureSize( 1 );
        const int uvDataSize =  uvSize.width() *  uvSize.height();
        _data[0] = QByteArray{ int(width * height), Qt::Uninitialized };
        _data[1] = QByteArray{ uvDataSize, Qt::Uninitialized };
        _data[2] = QByteArray{ uvDataSize, Qt::Uninitialized };
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

const uint8_t* FFMPEGPicture::getData( const uint texture ) const
{
    if( texture >= _data.size( ))
        return nullptr;

    return reinterpret_cast<const uint8_t*>( _data[texture].constData( ));
}

TextureFormat FFMPEGPicture::getFormat() const
{
    return _format;
}

uint8_t* FFMPEGPicture::getData( const uint texture )
{
    if( texture >= _data.size( ))
        return nullptr;

    return reinterpret_cast<uint8_t*>( _data[texture].data( ));
}

size_t FFMPEGPicture::getDataSize( const uint texture ) const
{
    if( texture >= _data.size( ))
        return 0;

    return _data[texture].size();
}

QImage FFMPEGPicture::toQImage() const
{
    if( getFormat() != TextureFormat::rgba )
        return QImage();

    return QImage( getData(), getWidth(), getHeight(),
                   QImage::Format_RGBA8888 );
}
