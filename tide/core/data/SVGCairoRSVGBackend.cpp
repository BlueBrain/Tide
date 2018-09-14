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

#include <librsvg/rsvg.h> // Must come before any Qt include

#include "SVGCairoRSVGBackend.h"

#include "CairoWrappers.h"

#include <mutex>

#define RSVG_HAS_LARGE_FILE_SUPPORT LIBRSVG_CHECK_VERSION(2, 40, 3)

struct RsvgHandleDeleter
{
    void operator()(RsvgHandle* rsvg) { g_object_unref(rsvg); }
};
typedef std::unique_ptr<RsvgHandle, RsvgHandleDeleter> RsvgHandlePtr;

struct GInputStreamDeleter
{
    void operator()(GInputStream* rsvg) { g_object_unref(rsvg); }
};
typedef std::unique_ptr<GInputStream, GInputStreamDeleter> GInputStreamPtr;

struct SVGCairoRSVGBackend::Impl
{
    RsvgHandlePtr svg;
    std::mutex renderMutex;
};

SVGCairoRSVGBackend::SVGCairoRSVGBackend(const QByteArray& svgData)
    : _impl{new Impl}
{
#if RSVG_HAS_LARGE_FILE_SUPPORT
    const auto flags = RSVG_HANDLE_FLAG_UNLIMITED;
#else
    const auto flags = RSVG_HANDLE_FLAGS_NONE;
#endif
    _impl->svg.reset(rsvg_handle_new_with_flags(flags));
    GInputStreamPtr input(
        g_memory_input_stream_new_from_data(svgData.constData(), svgData.size(),
                                            nullptr));
    GError* gerror = nullptr;
    if (!rsvg_handle_read_stream_sync(_impl->svg.get(), input.get(), nullptr,
                                      &gerror))
        throw std::runtime_error(gerror->message);
}

SVGCairoRSVGBackend::~SVGCairoRSVGBackend()
{
}

QSize SVGCairoRSVGBackend::getSize() const
{
    RsvgDimensionData dimensions;
    rsvg_handle_get_dimensions(_impl->svg.get(), &dimensions);
    return QSize{dimensions.width, dimensions.height};
}

QImage SVGCairoRSVGBackend::renderToImage(const QSize& imageSize,
                                          const QRectF& region) const
{
    const auto svgSize = QSizeF(getSize());

    const auto zoomX = 1.0 / region.width();
    const auto zoomY = 1.0 / region.height();

    const auto resX = imageSize.width() / svgSize.width();
    const auto resY = imageSize.height() / svgSize.height();

    const auto topLeft =
        QPointF{region.x() * svgSize.width(), region.y() * svgSize.height()};

    auto image = QImage{imageSize, QImage::Format_ARGB32_Premultiplied};
    image.fill(Qt::transparent);
    auto surface = CairoSurfacePtr{
        cairo_image_surface_create_for_data(image.bits(), CAIRO_FORMAT_ARGB32,
                                            image.width(), image.height(),
                                            4 * image.width())};
    auto context = CairoPtr{cairo_create(surface.get())};

    cairo_scale(context.get(), zoomX * resX, zoomY * resY);
    cairo_translate(context.get(), -topLeft.x(), -topLeft.y());

    const std::lock_guard<std::mutex> lock(_impl->renderMutex);
    rsvg_handle_render_cairo(_impl->svg.get(), context.get());

    return image;
}
