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

#include "PDFPopplerQtBackend.h"

#include <exception>

#include <poppler-qt5.h>

namespace
{
const qreal PDF_RES = 72.0;
}

PDFPopplerQtBackend::PDFPopplerQtBackend(const QString& uri)
    : _pdfDoc(Poppler::Document::load(uri))
{
    if (!_pdfDoc || _pdfDoc->isLocked() || !setPage(0))
        throw std::runtime_error("Could not open document");
}

PDFPopplerQtBackend::~PDFPopplerQtBackend()
{
}

QSize PDFPopplerQtBackend::getSize() const
{
    return _pdfPage->pageSize();
}

int PDFPopplerQtBackend::getPageCount() const
{
    return _pdfDoc->numPages();
}

bool PDFPopplerQtBackend::setPage(const int pageNumber)
{
    Poppler::Page* page = _pdfDoc->page(pageNumber);
    if (!page)
        return false;

    _pdfPage.reset(page);
    return true;
}

QImage PDFPopplerQtBackend::renderToImage(const QSize& imageSize,
                                          const QRectF& region) const
{
    const QSize pageSize(_pdfPage->pageSize());

    const qreal zoomX = 1.0 / region.width();
    const qreal zoomY = 1.0 / region.height();

    const QPointF topLeft(region.x() * imageSize.width(),
                          region.y() * imageSize.height());

    const qreal resX = PDF_RES * imageSize.width() / pageSize.width();
    const qreal resY = PDF_RES * imageSize.height() / pageSize.height();

    _pdfDoc->setRenderHint(Poppler::Document::TextAntialiasing);
    return _pdfPage->renderToImage(resX * zoomX, resY * zoomY,
                                   topLeft.x() * zoomX, topLeft.y() * zoomY,
                                   imageSize.width(), imageSize.height());
}
