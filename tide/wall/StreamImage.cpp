/*********************************************************************/
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project                  */
/*                          Daniel.Nachbaur@epfl.ch                  */
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

#include "StreamImage.h"

#include <deflect/Frame.h>

StreamImage::StreamImage( deflect::FramePtr frame, const uint tileIndex )
    : _frame{ frame }
    , _tileIndex{ tileIndex }
{}

int StreamImage::getWidth() const
{
    return _frame->segments.at( _tileIndex ).parameters.width;
}

int StreamImage::getHeight() const
{
    return _frame->segments.at( _tileIndex ).parameters.height;
}

const uint8_t* StreamImage::getData( const uint texture ) const
{
    const auto data = _frame->segments.at( _tileIndex ).imageData.constData();
    if( getFormat() == TextureFormat::rgba || texture == 0 )
        return reinterpret_cast<const uint8_t*>( data );

    size_t offset = getWidth() * getHeight();

    if( texture == 1 )
        return reinterpret_cast<const uint8_t*>( data ) + offset;

    if( texture == 2 )
    {
        const auto uvSize = getTextureSize( 1 );
        offset += uvSize.width() * uvSize.height();
        return reinterpret_cast<const uint8_t*>( data ) + offset;
    }
    return nullptr;
}

TextureFormat StreamImage::getFormat() const
{
    switch( _frame->segments.at( _tileIndex ).parameters.dataType )
    {
    case deflect::DataType::rgba:
        return TextureFormat::rgba;
    case deflect::DataType::yuv444:
        return TextureFormat::yuv444;
    case deflect::DataType::yuv422:
        return TextureFormat::yuv422;
    case deflect::DataType::yuv420:
        return TextureFormat::yuv420;
    case deflect::DataType::jpeg:
    default:
        throw std::runtime_error( "StreamImage texture is not decompressed" );
    }
}
