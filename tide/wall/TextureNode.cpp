/*********************************************************************/
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project                  */
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

#include "TextureNode.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQuickWindow>

TextureNode::TextureNode( const QSize& size, QQuickWindow* window,
                          TextureFormat )
    : _window( window )
    , _frontTexture( window->createTextureFromId( 0 , QSize( 1 ,1 )))
    , _backTexture( _createTexture( size ))
{
    setTexture( _frontTexture.get( ));
    setFiltering( QSGTexture::Linear );
    setMipmapFiltering( QSGTexture::Linear );
}

void TextureNode::setMipmapFiltering( const QSGTexture::Filtering filtering_ )
{
    auto mat = static_cast<QSGOpaqueTextureMaterial*>( material( ));
    auto opaqueMat = static_cast<QSGOpaqueTextureMaterial*>( opaqueMaterial( ));

    mat->setMipmapFiltering( filtering_ );
    opaqueMat->setMipmapFiltering( filtering_ );
}

uint TextureNode::getBackGlTexture() const
{
    return _backTexture->textureId();
}

void TextureNode::swap()
{
    std::swap( _frontTexture, _backTexture );

    setTexture( _frontTexture.get( ));
    markDirty( DirtyMaterial );
}

void TextureNode::prepareBackTexture( const QSize& size, TextureFormat )
{
    if( _backTexture->textureSize() == size )
        return;

    _backTexture = _createTexture( size );
}

TextureNode::QSGTexturePtr
TextureNode::_createTexture( const QSize& size ) const
{
    uint textureID;
    auto gl = QOpenGLContext::currentContext()->functions();
    gl->glActiveTexture( GL_TEXTURE0 );
    gl->glGenTextures( 1, &textureID );
    gl->glBindTexture( GL_TEXTURE_2D, textureID );
    gl->glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, size.width(), size.height(),
                      0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
    gl->glBindTexture( GL_TEXTURE_2D, 0 );

    return _createWrapper( textureID, size );
}

TextureNode::QSGTexturePtr
TextureNode::_createWrapper( const uint textureID, const QSize& size ) const
{
    const auto textureFlags = QQuickWindow::CreateTextureOptions(
                                  QQuickWindow::TextureHasAlphaChannel |
                                  QQuickWindow::TextureHasMipmaps |
                                  QQuickWindow::TextureOwnsGLTexture );
    return QSGTexturePtr( _window->createTextureFromId( textureID, size,
                                                        textureFlags ));
}
