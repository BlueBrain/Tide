/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
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

#include "PDF.h"

#include "log.h"
#include <poppler-qt5.h>

namespace
{
const int INVALID_PAGE_NUMBER = -1;
const qreal PDF_RES = 72.0;
}

PDF::PDF( const QString& uri )
    : _pdfDoc( 0 )
    , _pdfPage( 0 )
    , _pageNumber( INVALID_PAGE_NUMBER )
{
    _openDocument( uri );
}

PDF::~PDF()
{
    _closeDocument();
}

const QString& PDF::getFilename() const
{
    return _filename;
}

bool PDF::isValid() const
{
    return ( _pdfDoc != 0 );
}

QSize PDF::getSize() const
{
    return _pdfPage ? _pdfPage->pageSize() : QSize();
}

int PDF::getPage() const
{
    return _pageNumber;
}

void PDF::setPage( const int pageNumber )
{
    if( pageNumber == _pageNumber || !isValid( pageNumber ))
        return;

    Poppler::Page* page = _pdfDoc->page( pageNumber );
    if( !page )
    {
        put_flog( LOG_WARN, "Could not open page: %d in PDF document: '%s'",
                  pageNumber, _filename.toLocal8Bit().constData( ));
        return;
    }

    _closePage();
    _pdfPage = page;
    _pageNumber = pageNumber;
}

int PDF::getPageCount() const
{
    return _pdfDoc->numPages();
}

QImage PDF::renderToImage( const QSize& imageSize, const QRectF& region ) const
{
    const QSize pageSize( _pdfPage->pageSize( ));

    const qreal zoomX = 1.0 / region.width();
    const qreal zoomY = 1.0 / region.height();

    const QPointF topLeft( region.x() * imageSize.width(),
                           region.y() * imageSize.height( ));

    const qreal resX = PDF_RES * imageSize.width() / pageSize.width();
    const qreal resY = PDF_RES * imageSize.height() / pageSize.height();

    return _pdfPage->renderToImage( resX * zoomX, resY * zoomY,
                                    topLeft.x() * zoomX, topLeft.y() * zoomY,
                                    imageSize.width(), imageSize.height( ));
}

bool PDF::isValid( const int pageNumber ) const
{
    return pageNumber >=0 && pageNumber < _pdfDoc->numPages();
}

void PDF::_openDocument( const QString& filename )
{
    _closeDocument();

    _pdfDoc = Poppler::Document::load( filename );
    if ( !_pdfDoc || _pdfDoc->isLocked( ))
    {
        put_flog( LOG_DEBUG, "Could not open document: '%s'",
                  filename.toLocal8Bit().constData( ));
        _closeDocument();
        return;
    }

    _filename = filename;
    _pdfDoc->setRenderHint( Poppler::Document::TextAntialiasing );

    setPage( 0 );
}

void PDF::_closeDocument()
{
    if( _pdfDoc )
    {
        _closePage();
        delete _pdfDoc;
        _pdfDoc = 0;
        _filename.clear();
    }
}

void PDF::_closePage()
{
    if( _pdfPage )
    {
        delete _pdfPage;
        _pdfPage = 0;
        _pageNumber = INVALID_PAGE_NUMBER;
    }
}
