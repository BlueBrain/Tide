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

#include "TextureUploader.h"

#include "Tile.h"
#include "data/Image.h"
#include "log.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

TextureUploader::TextureUploader()
{
}

TextureUploader::~TextureUploader()
{
}

void TextureUploader::init(QOpenGLContext* shareContext, QScreen* screen)
{
    QMetaObject::invokeMethod(this, "_createGLContext",
                              Qt::BlockingQueuedConnection,
                              Q_ARG(QOpenGLContext*, shareContext),
                              Q_ARG(QScreen*, screen));

    // OSX: The offscreen surface must be created on the main/GUI thread
    // because it is backed by a real window.

    _offscreenSurface.reset(new QOffscreenSurface);
    _offscreenSurface->setFormat(_glContext->format());
    _offscreenSurface->setScreen(screen); // needed for multiple X displays
    _offscreenSurface->create();
}

void TextureUploader::stop()
{
    // OSX: The offscreen surface must be deleted on the main/GUI thread
    // because it is backed by a real window.

    QMetaObject::invokeMethod(this, "_deleteGLContext",
                              Qt::BlockingQueuedConnection);
    _offscreenSurface.reset();
}

void TextureUploader::_createGLContext(QOpenGLContext* shareContext,
                                       QScreen* screen)
{
    _glContext.reset(new QOpenGLContext);
    _glContext->setShareContext(shareContext);
    _glContext->setScreen(screen); // needed for multiple X displays
    _glContext->create();
}

void TextureUploader::_deleteGLContext()
{
    _glContext.reset();
}

void TextureUploader::uploadTexture(ImagePtr image, TileWeakPtr tile_)
{
    if (!image)
    {
        put_flog(LOG_DEBUG, "Invalid image");
        return;
    }

    TilePtr tile = tile_.lock();
    if (!tile)
    {
        put_flog(LOG_DEBUG, "Tile expired");
        return;
    }

    _glContext->makeCurrent(_offscreenSurface.get());

    if (image->isGpuImage() && !image->generateGpuImage())
    {
        put_flog(LOG_DEBUG, "GPU image generation failed");
        return;
    }

    if (image->getWidth() != tile->getBackGlTextureSize().width() ||
        image->getHeight() != tile->getBackGlTextureSize().height())
    {
        put_flog(LOG_DEBUG, "Incompatible image dimensions");
        return;
    }

    if (image->getFormat() != tile->getFormat())
    {
        put_flog(LOG_DEBUG, "Incompatible texture formats");
        return;
    }

    if (!_upload(*image, *tile))
        return;

    // notify tile that its texture has been updated
    tile->textureUpdated(tile);

    // notify RenderController for redraw
    emit uploaded();
}

bool TextureUploader::_upload(const Image& image, const Tile& tile)
{
    switch (image.getFormat())
    {
    case TextureFormat::rgba:
    {
        const auto textureID = tile.getBackGlTexture();
        if (!textureID)
        {
            put_flog(LOG_DEBUG, "Tile has no backTextureID");
            return false;
        }
        _upload(image, 0, textureID);
        return true;
    }
    case TextureFormat::yuv444:
    case TextureFormat::yuv422:
    case TextureFormat::yuv420:
    {
        const auto& texture = tile.getBackGlTextureYUV();
        if (!texture.y || !texture.u || !texture.v)
        {
            put_flog(LOG_DEBUG, "Tile is missing a back GL texture");
            return false;
        }
        _upload(image, 0, texture.y);
        _upload(image, 1, texture.u);
        _upload(image, 2, texture.v);
        return true;
    }
    default:
        put_flog(LOG_DEBUG, "image has unsupported texture format");
        return false;
    }
}

void TextureUploader::_upload(const Image& image, const uint srcTextureIdx,
                              const uint textureID)
{
    const auto textureSize = image.getTextureSize(srcTextureIdx);
    if (!textureSize.isValid())
    {
        put_flog(LOG_ERROR, "image texture has invalid size");
        return;
    }

    auto gl = _glContext->functions();

    GLint alignment = 1;
    if ((textureSize.width() % 4) == 0)
        alignment = 4;
    else if ((textureSize.width() % 2) == 0)
        alignment = 2;
    gl->glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

    gl->glBindTexture(GL_TEXTURE_2D, textureID);
    gl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureSize.width(),
                        textureSize.height(), image.getGLPixelFormat(),
                        GL_UNSIGNED_BYTE, image.getData(srcTextureIdx));
    gl->glGenerateMipmap(GL_TEXTURE_2D);
    gl->glBindTexture(GL_TEXTURE_2D, 0);

    // Ensure the texture upload is complete before the render thread uses it
    gl->glFinish();
}
