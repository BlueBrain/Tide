/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
/*                     Daniel.Nachbaur@epfl.ch                       */
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

#include "TextureUploader.h"

#include "MovieUpdater.h"
#include "Image.h"
#include "Tile.h"

#include "log.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

TextureUploader::TextureUploader()
    : _glContext( nullptr )
    , _offscreenSurface( nullptr )
    , _gl( nullptr )
    , _pbo( 0 )
    , _bufferSize( 0 )
{
    connect( this, &TextureUploader::init, this,
             &TextureUploader::_onInit, Qt::BlockingQueuedConnection );
    connect( this, &TextureUploader::stop, this,
             &TextureUploader::_onStop, Qt::BlockingQueuedConnection );
}

void TextureUploader::_onInit( QOpenGLContext* shareContext )
{
    _glContext = new QOpenGLContext;
    _glContext->setShareContext( shareContext );
    _glContext->create();

    _offscreenSurface = new QOffscreenSurface;
    _offscreenSurface->setFormat( _glContext->format( ));
    _offscreenSurface->create();

    _glContext->makeCurrent( _offscreenSurface );
    _gl = _glContext->functions();

    _gl->glGenBuffers( 1, &_pbo );
}

void TextureUploader::_onStop()
{
    _glContext->makeCurrent( _offscreenSurface );

    _gl->glDeleteBuffers( 1, &_pbo );

    delete _offscreenSurface;
    delete _glContext;
}

void TextureUploader::uploadTexture( ImagePtr image, TileWeakPtr tile_ )
{
    if( !image )
    {
        put_flog( LOG_DEBUG, "Invalid image" );
        return;
    }

    TilePtr tile = tile_.lock();
    if( !tile )
    {
        put_flog( LOG_DEBUG, "Tile expired" );
        return;
    }

    _glContext->makeCurrent( _offscreenSurface );

    if( image->isGpuImage() && !image->generateGpuImage( ))
    {
        put_flog( LOG_DEBUG, "GPU image generation failed" );
        return;
    }

    if( image->getWidth() != tile->getBackGlTextureSize().width() ||
        image->getHeight() != tile->getBackGlTextureSize().height( ))
    {
        put_flog( LOG_DEBUG, "Incompatible image dimensions!" );
        return;
    }

    const uint textureID = tile->getBackGlTexture();
    if( !textureID )
    {
        put_flog( LOG_DEBUG, "Tile has no backTextureID" );
        return;
    }

    _upload( *image, textureID );

    // notify tile that its texture has been updated
    tile->textureUpdated( tile );

    // notify RenderController for redraw
    emit uploaded();
}

void TextureUploader::_upload( const Image& image, const uint textureID )
{
    // we observed slow-downs with movie session and pixelstreams together
    // which originates from glFinish() in upload & render thread. For now
    // we use 'slow&easy' texture upload. This probably points out the problem
    // and a potential solution:
    // http://stackoverflow.com/questions/31941385/opengl-driver-seems-to-be-ridiculously-slow-from-glunmapbuffer-this-cant-be-ri
#if 0
    // make PBO big enough
    _gl->glBindBuffer( GL_PIXEL_UNPACK_BUFFER, _pbo );
    const size_t bufferSize = image.getSize();
    if( bufferSize > _bufferSize )
    {
        _gl->glBufferData( GL_PIXEL_UNPACK_BUFFER, bufferSize, 0,
                           GL_STREAM_DRAW );
        _bufferSize = bufferSize;
    }

    // copy pixels from CPU mem to GPU mem
    void* pboData = _gl->glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY );
    std::memcpy( pboData, image.getData(), bufferSize );
    _gl->glUnmapBuffer( GL_PIXEL_UNPACK_BUFFER );

    // setup PBO and texture pixel storage
    GLint alignment = 1;
    if( (image.getWidth() % 4) == 0 )
        alignment = 4;
    else if( (image.getWidth() % 2) == 0 )
        alignment = 2;
    _gl->glPixelStorei( GL_UNPACK_ALIGNMENT, alignment );
    _gl->glPixelStorei( GL_UNPACK_ROW_LENGTH, image.getWidth( ));

    // update texture with pixels from PBO
    _gl->glActiveTexture( GL_TEXTURE0 );
    _gl->glBindTexture( GL_TEXTURE_2D, textureID );
    _gl->glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, image.getWidth(),
                          image.getHeight(), image.getFormat(),
                          GL_UNSIGNED_BYTE, 0 );

    _gl->glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
#else
    GLint alignment = 1;
    if( (image.getWidth() % 4) == 0 )
        alignment = 4;
    else if( (image.getWidth() % 2) == 0 )
        alignment = 2;
    _gl->glPixelStorei( GL_UNPACK_ALIGNMENT, alignment );

    _gl->glActiveTexture( GL_TEXTURE0 );
    _gl->glBindTexture( GL_TEXTURE_2D, textureID );
    _gl->glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, image.getWidth(),
                          image.getHeight(), image.getFormat(),
                          GL_UNSIGNED_BYTE, image.getData( ));
#endif
    _gl->glGenerateMipmap( GL_TEXTURE_2D );

    _gl->glBindTexture( GL_TEXTURE_2D, 0 );

    // Ensure the texture upload is complete before the render thread uses it
    _gl->glFinish();
}
