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

#include "PDFTiler.h"

#include "VectorialContent.h"
#include "LodTools.h"

#include <QThread>

namespace
{
const uint tileSize = 512;
}

PDFTiler::PDFTiler( PDF& pdf )
    : LodTiler( pdf.getSize() * VectorialContent::getMaxScale(), tileSize )
    , _pdf( pdf )
    , _tilesPerPage( _lodTool.getTilesCount( ))
{}

QRect PDFTiler::getTileRect( uint tileId ) const
{
    tileId = tileId % _tilesPerPage;
    return LodTiler::getTileRect( tileId );
}

Indices PDFTiler::computeVisibleSet( const QRectF& visibleTilesArea,
                                     const uint lod ) const
{
    const Indices visibleSet = _lodTool.getVisibleTiles( visibleTilesArea,
                                                         lod );
    Indices offsetSet;
    const auto pageOffset = getPreviewTileId();
    for( auto tileId : visibleSet )
        offsetSet.insert( tileId + pageOffset );

    return offsetSet;
}

QImage PDFTiler::getCachableTileImage( uint tileId ) const
{
    const auto id = QThread::currentThreadId();

    PDF* pdf = nullptr;
    {
        QMutexLocker lock( &_threadMapMutex );
        if( !_perThreadPDF.count( id ))
            _perThreadPDF[id] = make_unique<PDF>( _pdf.getFilename( ));
        pdf = _perThreadPDF[id].get();
    }
    pdf->setPage( tileId / _tilesPerPage );

    tileId = tileId % _tilesPerPage;
    const QRect tile = getTileRect( tileId );
    return pdf->renderToImage( tile.size(), getNormalizedTileRect( tileId ));
}

uint PDFTiler::getPreviewTileId() const
{
    return _tilesPerPage * _pdf.getPage();
}
