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

#include "FFMPEGVideoFrameConverter.h"

#include "FFMPEGFrame.h"
#include "FFMPEGPicture.h"

#pragma clang diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "FFMPEGDefines.h"

extern "C"
{
    #include <libswscale/swscale.h>
}

AVPixelFormat _toAVPixelFormat( const TextureFormat format )
{
    switch( format )
    {
    case TextureFormat::rgba:
        return AV_PIX_FMT_RGBA;
    case TextureFormat::yuv420:
        return AV_PIX_FMT_YUV420P;
    case TextureFormat::yuv422:
        return AV_PIX_FMT_YUV422P;
    case TextureFormat::yuv444:
        return AV_PIX_FMT_YUV444P;
    default:
        throw std::logic_error( "FFMPEGPicture: unsupported format" );
    }
}

struct FFMPEGVideoFrameConverter::Impl
{
    SwsContext* swsContext = nullptr;
};

FFMPEGVideoFrameConverter::FFMPEGVideoFrameConverter()
    : _impl( new Impl )
{
}

FFMPEGVideoFrameConverter::~FFMPEGVideoFrameConverter()
{
    sws_freeContext( _impl->swsContext );
}

PicturePtr FFMPEGVideoFrameConverter::convert( const FFMPEGFrame& srcFrame,
                                               const TextureFormat format )
{
    auto picture = std::make_shared<FFMPEGPicture>( srcFrame.getWidth(),
                                                    srcFrame.getHeight(),
                                                    format );

    const auto avFormat = _toAVPixelFormat( format );

    _impl->swsContext = sws_getCachedContext( _impl->swsContext,
                                              srcFrame.getWidth(),
                                              srcFrame.getHeight(),
                                              srcFrame.getAVPixelFormat(),
                                              picture->getWidth(),
                                              picture->getHeight(),
                                              avFormat,
                                              SWS_FAST_BILINEAR,
                                              nullptr, nullptr, nullptr );
    if( !_impl->swsContext )
        return PicturePtr();

    uint8_t* dstData[3];
    int linesize[3];
    for( size_t i = 0; i < 3; ++i )
    {
        dstData[i] = picture->getData( i );
        // width of image plane in pixels * bytes per pixel
        linesize[i] = picture->getDataSize( i ) /
                      picture->getTextureSize( i ).height();
    }

    const auto outputHeight = sws_scale( _impl->swsContext,
                                         srcFrame.getAVFrame().data,
                                         srcFrame.getAVFrame().linesize,
                                         0, srcFrame.getHeight(),
                                         dstData, linesize );
    if( outputHeight != picture->getHeight( ))
        return PicturePtr();

    return picture;
}
