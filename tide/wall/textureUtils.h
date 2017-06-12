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

#ifndef TEXTUREUTILS_H
#define TEXTUREUTILS_H

#include "types.h"

class QOpenGLBuffer;
class QSGTexture;
class QQuickWindow;

/**
 * Texture utility functions.
 */
namespace textureUtils
{
/**
 * Create a 8-bit texture.
 *
 * @param size in pixels.
 * @param window the QQuickWindow needed to create a QSGTexture wrapper.
 * @return a QSGTexture owning its GL texture.
 */
std::unique_ptr<QSGTexture> createTexture(const QSize& size,
                                          QQuickWindow& window);
/**
 * Create a 32-bit RGBA texture.
 *
 * @param size in pixels.
 * @param window the QQuickWindow needed to create a QSGTexture wrapper.
 * @return a QSGTexture owning its GL texture.
 */
std::unique_ptr<QSGTexture> createTextureRgba(const QSize& size,
                                              QQuickWindow& window);

/**
 * Create a Pixel Buffer Object.
 *
 * @param dynamic true if the buffer is going to be updated frequently.
 * @return empty, ready-to-use pbo.
 */
std::unique_ptr<QOpenGLBuffer> createPbo(bool dynamic);

/**
 * Upload an image to a PBO.
 *
 * @param image the source image
 * @param srcTextureIdx the texture plane of the source image.
 * @param pbo the target PBO, will be resized to the image size.
 */
void upload(const Image& image, const uint srcTextureIdx, QOpenGLBuffer& pbo);

/**
 * Copy a PBO to a GPU texture.
 *
 * @param pbo the source PBO.
 * @param texture the target texture, must be of the same size as the PBO.
 */
void copy(QOpenGLBuffer& pbo, QSGTexture& texture, uint glTextFormat);
}

#endif
