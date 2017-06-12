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

#include <QSGNode>

class QQuickWindow;

/**
 * A node with a double buffered YUV texture.
 *
 * Initially it displays an empty black texture (id 0). Users can upload data
 * asynchronously to the texture and call swap() on the next frame rendering to
 * display the results.
 *
 * The texture can be either static or dynamic:
 * * In the dynamic case, two PBOs are used for real-time texture updates.
 * * In the static case, a single PBO is used for the initial texture upload and
 *   then released in the first call to swap() so that no memory is wasted.
 */
class TextureNodeYUV : public QSGNode
{
public:
    /**
     * Create a textured rectangle for rendering YUV images on the GPU.
     * @param window a reference to the quick window for generating textures.
     * @param dynamic true if the texture is going to be updated more than once.
     */
    TextureNodeYUV(QQuickWindow* window, bool dynamic);

    /** @return the surface of the node. */
    const QRectF& rect() const;

    /** Set the surface of the node. */
    void setRect(const QRectF& rect);

    /** Upload the given image to the back PBO. */
    void updateBackTexture(const Image& image);

    /** Swap the PBOs and update the texture with the back PBO's contents. */
    void swap();

private:
    QQuickWindow* _window = nullptr;
    bool _dynamicTexture = false;

    QRectF _rect;
    QSGGeometryNode _node;

    QSize _nextTextureSize;
    TextureFormat _nextFormat;

    bool _needTextureChange() const;
    void _createTextures(const QSize& size, TextureFormat format);
    void _createBackPbos();
    void _deletePbos();
    void _uploadToBackPbos(const Image& image);
    void _copyFrontPbosToTextures();
    void _swapPbos();
};

#endif
