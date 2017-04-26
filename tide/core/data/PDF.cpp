/*********************************************************************/
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
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

#include "PDF.h"

#include "log.h"

#if TIDE_USE_CAIRO && TIDE_USE_POPPLER_GLIB
#include "PDFPopplerCairoBackend.h"
#else
#include "PDFPopplerQtBackend.h"
#endif

namespace
{
const int INVALID_PAGE_NUMBER = -1;
}

std::unique_ptr<PDFBackend> _createPdfBackend(const QString& uri)
{
#if TIDE_USE_CAIRO && TIDE_USE_POPPLER_GLIB
    return make_unique<PDFPopplerCairoBackend>(uri);
#else
    return make_unique<PDFPopplerQtBackend>(uri);
#endif
}

struct PDF::Impl
{
    QString filename;
    int currentPage = 0;
    std::unique_ptr<PDFBackend> pdf;
};

PDF::PDF(const QString& uri)
    : _impl(new Impl)
{
    _impl->filename = uri;
    try
    {
        _impl->pdf = _createPdfBackend(uri);
    }
    catch (const std::runtime_error& e)
    {
        put_flog(LOG_DEBUG, "Could not open document '%s': '%s'",
                 uri.toLocal8Bit().constData(), e.what());
    }
}

PDF::~PDF()
{
}

const QString& PDF::getFilename() const
{
    return _impl->filename;
}

bool PDF::isValid() const
{
    return bool(_impl->pdf);
}

QSize PDF::getSize() const
{
    return isValid() ? _impl->pdf->getSize() : QSize();
}

int PDF::getPage() const
{
    return isValid() ? _impl->currentPage : INVALID_PAGE_NUMBER;
}

void PDF::setPage(const int pageNumber)
{
    if (pageNumber == getPage())
        return;
    if (pageNumber < 0 || pageNumber >= getPageCount())
        return;

    if (!_impl->pdf->setPage(pageNumber))
    {
        put_flog(LOG_WARN, "Could not open page: %d in PDF document: '%s'",
                 pageNumber, _impl->filename.toLocal8Bit().constData());
        return;
    }
    _impl->currentPage = pageNumber;
}

int PDF::getPageCount() const
{
    return isValid() ? _impl->pdf->getPageCount() : 0;
}

QImage PDF::renderToImage(const QSize& imageSize, const QRectF& region) const
{
    return isValid() ? _impl->pdf->renderToImage(imageSize, region) : QImage();
}
