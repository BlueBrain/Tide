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

#ifndef PDFTILER_H
#define PDFTILER_H

#include "LodTiler.h"

#include <QObject>

class PDF;

/**
 * Represent a PDF document as a multi-LOD tiled data source.
 */
class PDFTiler : public QObject, public LodTiler
{
    Q_OBJECT
    Q_DISABLE_COPY(PDFTiler)

public:
    /** Constructor. */
    explicit PDFTiler(const QString& uri, const QSize& maxImageSize);

    /** Destructor. */
    ~PDFTiler();

    /** @copydoc DataSource::getUri */
    QString getUri() const final;

    /** Update this datasource according to pdf content (set page info). */
    void update(const Content& content) final;

    /** @copydoc DataSource::getTileRect */
    QRect getTileRect(uint tileId) const final;

    /** @copydoc DataSource::computeVisibleSet */
    Indices computeVisibleSet(const QRectF& visibleTilesArea, uint lod,
                              uint channel) const final;

    /** @return the ID of the preview (lowest res.) tile for the current page */
    uint getPreviewTileId() const final;

    /** @return the current page / total number of pages of the document. */
    QString getStatistics() const;

signals:
    /** Emitted when the PDF page has changed. */
    void pageChanged();

private:
    QImage getCachableTileImage(uint tileId, deflect::View view) const final;
    bool isStereo() const final { return false; }
    const LodTools& _getLodTool() const final { return *_lodTool; }
    PDF& _getPdfForCurrentThread() const;

    const QString _uri;
    std::unique_ptr<LodTools> _lodTool;
    uint _tilesPerPage;
    int _currentPage = 0;
    int _pageCount = 0;

    mutable QMutex _threadMapMutex;
    mutable std::map<Qt::HANDLE, std::unique_ptr<PDF>> _perThreadPDF;
};

#endif
