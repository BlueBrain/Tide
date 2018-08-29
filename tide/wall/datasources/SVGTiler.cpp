/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
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

#include "SVGTiler.h"

#include "tools/LodTools.h"
#include "utils/log.h"

#if !(TIDE_USE_CAIRO && TIDE_USE_RSVG)
#include "SVGGpuImage.h"
#endif

#include <QThread>

namespace
{
const uint tileSize = 1024;
}

SVGTiler::SVGTiler(const QString& uri, const QSize& maxImageSize)
{
    try
    {
        _svg = std::make_unique<SVG>(uri);
        _lodTool = std::make_unique<LodTools>(maxImageSize, tileSize);
    }
    catch (const std::runtime_error& e)
    {
        _lodTool = std::make_unique<LodTools>(QSize(), 1u);
        print_log(LOG_WARN, LOG_CONTENT, "SVG error: %s - %s",
                  uri.toLocal8Bit().constData(), e.what());
    }
}

SVGTiler::~SVGTiler()
{
}

ImagePtr SVGTiler::getTileImage(const uint tileId,
                                const deflect::View view) const
{
#if !(TIDE_USE_CAIRO && TIDE_USE_RSVG)
    if (_svg && !contains(tileId))
        return std::make_shared<SVGGpuImage>(*this, tileId);
#endif
    return CachedDataSource::getTileImage(tileId, view);
}

#if !(TIDE_USE_CAIRO && TIDE_USE_RSVG)
ImagePtr SVGTiler::renderAndCacheGpuImage(const uint tileId,
                                          const deflect::View view) const
{
    return CachedDataSource::getTileImage(tileId, view);
}
#endif

QImage SVGTiler::getCachableTileImage(const uint tileId,
                                      const deflect::View view) const
{
    Q_UNUSED(view);

    if (!_svg)
        return QImage();

    const auto imageSize = getTileRect(tileId).size();
    const auto zoomRect = getNormalizedTileRect(tileId);

    return _getSvgForCurrentThread().renderToImage(imageSize, zoomRect);
}

SVG& SVGTiler::_getSvgForCurrentThread() const
{
#if TIDE_USE_CAIRO && TIDE_USE_RSVG
    // The SvgCairoRSVGBackend is called from multiple threads
    const auto id = QThread::currentThreadId();
    const QMutexLocker lock(&_threadMapMutex);
    if (!_perThreadSVG.count(id))
        _perThreadSVG[id] = std::make_unique<SVG>(_svg->getData());
    return *_perThreadSVG[id];
#else
    // The SvgQtGpuBackend is always called from the render thread (GPU)
    return *_svg;
#endif
}
