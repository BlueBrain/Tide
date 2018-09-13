/*********************************************************************/
/* Copyright (c) 2017-2018, EPFL/Blue Brain Project                  */
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

#include "textureUtils.h"

#include "data/Image.h"

#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQuickWindow>
#include <QSGTexture>

#include <cstring> // std::memcpy

namespace textureUtils
{
void upload(const Image& image, const uint srcTextureIdx, QOpenGLBuffer& pbo)
{
    pbo.bind();
    const auto size = image.getDataSize(srcTextureIdx);
    if (size_t(pbo.size()) != size)
        pbo.allocate(size);
    auto pboData = pbo.map(QOpenGLBuffer::WriteOnly);
    std::memcpy(pboData, image.getData(srcTextureIdx), size);
    pbo.unmap();
    pbo.release();
}

GLint _getUnpackAlignment(const uint textureWidth)
{
    if (textureWidth % 4 == 0)
        return 4;
    if (textureWidth % 2 == 0)
        return 2;
    return 1;
}

void copy(QOpenGLBuffer& pbo, QSGTexture& texture, const uint glTexFormat)
{
    auto gl = QOpenGLContext::currentContext()->functions();

    const auto textureSize = texture.textureSize();

    gl->glPixelStorei(GL_UNPACK_ALIGNMENT,
                      _getUnpackAlignment(textureSize.width()));

    texture.bind();
    pbo.bind();
    gl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureSize.width(),
                        textureSize.height(), glTexFormat, GL_UNSIGNED_BYTE, 0);
    pbo.release();
    gl->glGenerateMipmap(GL_TEXTURE_2D);
}

std::unique_ptr<QSGTexture> createTexture(const QSize& size,
                                          QQuickWindow& window)
{
    auto gl = QOpenGLContext::currentContext()->functions();

    auto textureID = GLuint{0};
    gl->glGenTextures(1, &textureID);
    gl->glBindTexture(GL_TEXTURE_2D, textureID);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, size.width(), size.height(), 0,
                     GL_RED, GL_UNSIGNED_BYTE, nullptr);

    const auto textureFlags =
        QQuickWindow::CreateTextureOptions(QQuickWindow::TextureOwnsGLTexture);
    return std::unique_ptr<QSGTexture>{
        window.createTextureFromId(textureID, size, textureFlags)};
}

std::unique_ptr<QSGTexture> createTextureRgba(const QSize& size,
                                              QQuickWindow& window)
{
    auto gl = QOpenGLContext::currentContext()->functions();

    auto textureID = GLuint{0};
    gl->glActiveTexture(GL_TEXTURE0);
    gl->glGenTextures(1, &textureID);
    gl->glBindTexture(GL_TEXTURE_2D, textureID);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.width(), size.height(), 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, 0);
    gl->glBindTexture(GL_TEXTURE_2D, 0);

    const auto textureFlags = QQuickWindow::CreateTextureOptions(
        QQuickWindow::TextureOwnsGLTexture |
        QQuickWindow::TextureHasAlphaChannel);
    return std::unique_ptr<QSGTexture>{
        window.createTextureFromId(textureID, size, textureFlags)};
}

std::unique_ptr<QOpenGLBuffer> createPbo(const bool dynamic)
{
    auto pbo =
        std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::PixelUnpackBuffer);
    pbo->create();
    pbo->setUsagePattern(dynamic ? QOpenGLBuffer::DynamicDraw
                                 : QOpenGLBuffer::StaticDraw);
    return pbo;
}
}
