/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
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

#ifndef TEXTURENODEYUV_H
#define TEXTURENODEYUV_H

#include "types.h"
#include "YUVTexture.h"
#include "yuv.h"

#include <QSGNode>
#include <QSGTexture>

class QQuickWindow;

/**
 * A node with a double buffered YUV texture.
 *
 * Initially it displays an empty black texture (id 0). Users can upload data
 * to the back texture, querried with getBackGlTextureYUV(), before calling
 * swap() to display the results.
 *
 * The second texture is created only after a call to setBackTextureSize(), so
 * that no memory is wasted for a second texture if the node is not going to
 * be updated more than once.
 */
class TextureNodeYUV : public QObject, public QSGNode
{
    Q_OBJECT

public:
    /**
     * Create a textured rectangle for rendering YUV images on the GPU.
     * @param size the initial back texture size.
     * @param window a reference to the quick window for generating textures.
     * @param format the YUV format to determine the U and V texture size.
     */
    TextureNodeYUV( const QSize& size, QQuickWindow* window,
                    TextureFormat format );

    /** @return the surface of the node. */
    const QRectF& rect() const;

    /** Set the surface of the node. */
    void setRect( const QRectF& rect );

    /** @return the back texture identifiers, which can safely be updated. */
    YUVTexture getBackGlTexture() const;

    /** Swap the front and back textures. */
    void swap();

    /**
     * Create or resize the back texture as needed.
     * Note that the back texture identifier may change as a result of calling
     * this function.
     * @param size the new texture size
     */
    void setBackTextureSize( const QSize& size );

private:
    QQuickWindow* _window = nullptr;
    const TextureFormat _format = TextureFormat::yuv420;

    QRectF _rect;
    QSGGeometryNode _node;

    void _createBackTextures( const QSize& size );
    using QSGTexturePtr = std::unique_ptr<QSGTexture>;
    QSGTexturePtr _createTexture( const QSize& size ) const;
    QSGTexturePtr _createWrapper( uint textureID, const QSize& size ) const;
};

#endif
