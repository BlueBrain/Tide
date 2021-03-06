/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#include "SVGQtGpuBackend.h"

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>
#include <QPainter>

#include <memory>

namespace
{
const int MULTI_SAMPLE_ANTI_ALIASING_SAMPLES = 8;
}

SVGQtGpuBackend::SVGQtGpuBackend(const QByteArray& svgData)
{
    if (!_svgRenderer.load(svgData) || !_svgRenderer.isValid())
        throw std::runtime_error("svg data could not be loaded");
}

QSize SVGQtGpuBackend::getSize() const
{
    return _svgRenderer.defaultSize();
}

void _saveGLState()
{
#ifndef __APPLE__
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
#endif
}

void _restoreGLState()
{
#ifndef __APPLE__
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
#endif
}

std::unique_ptr<QOpenGLFramebufferObject> _createMultisampledFBO(
    const QSize& size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(MULTI_SAMPLE_ANTI_ALIASING_SAMPLES);

    auto fbo = std::make_unique<QOpenGLFramebufferObject>(size, format);
    fbo->bind();
    auto gl = QOpenGLContext::currentContext()->functions();
    gl->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT);
    fbo->release();

    return fbo;
}

QRectF _getViewBox(const QRectF& zoomRect, const QRectF& svgExtents)
{
    return QRectF(svgExtents.x() + zoomRect.x() * svgExtents.width(),
                  svgExtents.y() + zoomRect.y() * svgExtents.height(),
                  zoomRect.width() * svgExtents.width(),
                  zoomRect.height() * svgExtents.height());
}

QImage SVGQtGpuBackend::renderToImage(const QSize& imageSize,
                                      const QRectF& region) const
{
    _saveGLState();

    // Use a separate multisampled FBO for anti-aliased rendering
    auto renderFbo = _createMultisampledFBO(imageSize);
    renderFbo->bind();

    // the paint device acts on the currently bound fbo
    QOpenGLPaintDevice device(renderFbo->size());
    QPainter painter(&device);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    {
        const QMutexLocker lock(&_mutex);
        const QRectF viewBoxBackup = _svgRenderer.viewBoxF();
        _svgRenderer.setViewBox(_getViewBox(region, viewBoxBackup));
        _svgRenderer.render(&painter);
        painter.end();
        _svgRenderer.setViewBox(viewBoxBackup);
    }

    renderFbo->release();
    _restoreGLState();

    // Blit to target texture FBO
    const QRect blitRect(0, 0, renderFbo->width(), renderFbo->height());
    QOpenGLFramebufferObject targetFbo(renderFbo->size());
    QOpenGLFramebufferObject::blitFramebuffer(&targetFbo, blitRect,
                                              renderFbo.get(), blitRect);

    return targetFbo.toImage();
}
