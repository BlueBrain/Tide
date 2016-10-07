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

#include "ImagePyramidDataSource.h"

#include "data/TiffPyramidReader.h"

namespace
{
const QSize previewSize{ 1920, 1920 };
}

std::pair<QSize, uint> _getLodParameters( const QString& uri )
{
    const TiffPyramidReader tif{ uri };
    const QSize tileSize = tif.getTileSize();
    if( tileSize.width() != tileSize.height( ))
        throw std::runtime_error( "Non-square tiles are not supported" );
    return std::make_pair( tif.getImageSize(), tileSize.width( ));
}

ImagePyramidDataSource::ImagePyramidDataSource( const QString& uri )
    : LodTiler{ _getLodParameters( uri )}
    , _uri{ uri }
{}


QRect ImagePyramidDataSource::getTileRect( const uint tileId ) const
{
    if( tileId == 0 )
    {
        TiffPyramidReader tif{ _uri };
        return { QPoint(), tif.readSize( tif.findLevel( previewSize )) };
    }
    return LodTiler::getTileRect( tileId );
}

QImage ImagePyramidDataSource::getCachableTileImage( const uint tileId ) const
{
    TiffPyramidReader tif{ _uri };

    QImage image;
    if( tileId == 0 )
        image = tif.readImage( tif.findLevel( previewSize ));
    else
    {
        const auto index = _lodTool.getTileIndex( tileId );
        image = tif.readTile( index.x, index.y, index.lod );
    }

    // TIFF tiles all have a fixed size. Those at the top of the pyramid
    // (or at the borders) are padded, but Tide expects to get tiles of the
    // exact dimensions (not padding) for uploading as GL textures.
    const QSize expectedSize = getTileRect( tileId ).size();
    if( image.size() != expectedSize )
        image = image.copy( QRect( QPoint(), expectedSize ));

    // Tide currently only supports 32 bit textures, convert if needed.
    if( image.pixelFormat().bitsPerPixel() != 32 )
        image = image.convertToFormat( QImage::Format_RGB32 );
    return image;
}
