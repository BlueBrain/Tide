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

#include "TextureNodeRGBA.h"

#include "data/Image.h"
#include "textureUtils.h"

#include <QQuickWindow>

TextureNodeRGBA::TextureNodeRGBA(QQuickWindow& window, const bool dynamic)
    : _window(window)
    , _dynamicTexture(dynamic)
    , _texture(window.createTextureFromId(0, QSize(1, 1)))
{
    if (_texture) // needed for null texture in unit tests without a scene graph
        setTexture(_texture.get());
    setFiltering(QSGTexture::Linear);
    setMipmapFiltering(QSGTexture::Linear);
}

void TextureNodeRGBA::setMipmapFiltering(const QSGTexture::Filtering filtering_)
{
    auto mat = static_cast<QSGOpaqueTextureMaterial*>(material());
    auto opaqueMat = static_cast<QSGOpaqueTextureMaterial*>(opaqueMaterial());

    mat->setMipmapFiltering(filtering_);
    opaqueMat->setMipmapFiltering(filtering_);
}

void TextureNodeRGBA::uploadTexture(const Image& image)
{
    if (!image.getTextureSize().isValid())
        throw std::runtime_error("image texture has invalid size");

    if (image.getRowOrder() == deflect::RowOrder::bottom_up)
        setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
    else
        setTextureCoordinatesTransform(QSGSimpleTextureNode::NoTransform);

    if (!_pbo)
        _pbo = textureUtils::createPbo(_dynamicTexture);

    textureUtils::upload(image, 0, *_pbo);

    _nextTextureSize = image.getTextureSize();
    _glImageFormat = image.getGLPixelFormat();
}

void TextureNodeRGBA::swap()
{
    if (_texture->textureSize() != _nextTextureSize)
        _texture = textureUtils::createTextureRgba(_nextTextureSize, _window);

    textureUtils::copy(*_pbo, *_texture, _glImageFormat);
    setTexture(_texture.get());
    markDirty(DirtyMaterial);

    if (!_dynamicTexture)
        _pbo.reset();
}
