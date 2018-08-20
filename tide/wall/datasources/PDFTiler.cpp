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

#include "PDFTiler.h"

#include "data/PDF.h"
#include "scene/PDFContent.h"
#include "tools/LodTools.h"
#include "utils/log.h"

#include <QThread>

namespace
{
// The main bottelneck of Poppler is the parsing done for every render call not
// the rendering itself. See: https://bugzilla.gnome.org/show_bug.cgi?id=303365
// Rendering a small tile takes almost as long a rendering the whole page, so
// it is more optimal to use a large tile size.
const uint tileSize = 2048;
}

PDFTiler::PDFTiler(const QString& uri, const QSize& maxImageSize)
    : _uri{uri}
{
    try
    {
        std::make_unique<PDF>(uri);
        _lodTool = std::make_unique<LodTools>(maxImageSize, tileSize);
    }
    catch (const std::runtime_error& e)
    {
        _lodTool = std::make_unique<LodTools>(QSize(), 1);
        print_log(LOG_WARN, LOG_CONTENT, "PDF error: %s - %s",
                  uri.toLocal8Bit().constData(), e.what());
    }
    _tilesPerPage = _lodTool->getTilesCount();
}

PDFTiler::~PDFTiler()
{
}

void PDFTiler::update(const Content& content)
{
    const auto& pdf = dynamic_cast<const PDFContent&>(content);

    _pageCount = pdf.getPageCount();
    if (_currentPage != pdf.getPage())
    {
        _currentPage = pdf.getPage();
        emit pageChanged();
    }
}

QRect PDFTiler::getTileRect(const uint tileId) const
{
    return LodTiler::getTileRect(tileId % _tilesPerPage);
}

Indices PDFTiler::computeVisibleSet(const QRectF& visibleTilesArea,
                                    const uint lod, const uint channel) const
{
    const auto pageOffset = getPreviewTileId();
    Indices offsetSet;
    for (auto tileId :
         LodTiler::computeVisibleSet(visibleTilesArea, lod, channel))
    {
        offsetSet.insert(tileId + pageOffset);
    }
    return offsetSet;
}

QImage PDFTiler::getCachableTileImage(const uint tileId,
                                      const deflect::View view) const
{
    Q_UNUSED(view);

    const auto id = QThread::currentThreadId();

    PDF* pdf = nullptr;
    try
    {
        QMutexLocker lock(&_threadMapMutex);
        if (!_perThreadPDF.count(id))
            _perThreadPDF[id] = std::make_unique<PDF>(_uri);
        pdf = _perThreadPDF[id].get();
    }
    catch (const std::runtime_error&)
    {
        return QImage();
    }
    pdf->setPage(tileId / _tilesPerPage);

    const auto imageSize = getTileRect(tileId).size();
    const auto region = LodTiler::getNormalizedTileRect(tileId % _tilesPerPage);
    return pdf->renderToImage(imageSize, region);
}

uint PDFTiler::getPreviewTileId() const
{
    return _tilesPerPage * _currentPage;
}

QString PDFTiler::getStatistics() const
{
    return QString("page %1/%2").arg(_currentPage + 1).arg(_pageCount);
}
