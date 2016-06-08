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

#include "ImageSynchronizer.h"

#include "QtImage.h"
#include "Tile.h"

#include <QImage>
#include <QImageReader>

ImageSynchronizer::ImageSynchronizer( const QString& uri )
    : _uri( uri )
{}

void ImageSynchronizer::update( const ContentWindow& window,
                                const QRectF& visibleArea )
{
    Q_UNUSED( window );

    // The error content stores the size of the original content, which is
    // generally different than the error image's size. But the tile must
    // always have the same size as the texture, otherwise the texture upload
    // fails.
    if( !visibleArea.isEmpty( ))
        createTile( QImageReader( _uri ).size( ));
}

ImagePtr ImageSynchronizer::getTileImage( const uint tileIndex ) const
{
    Q_UNUSED( tileIndex );

    QImage image( _uri );
    // Make sure image format is 32-bits per pixel as required by the GL texture
    if( !QtImage::is32Bits( image ))
        image = image.convertToFormat( QImage::Format_ARGB32 );
    if( image.isNull( ))
        return ImagePtr();
    return std::make_shared<QtImage>( image );
}

TilePtr ImageSynchronizer::getZoomContextTile() const
{
    const QRect rect( QPoint( 0, 0 ), QImageReader( _uri ).size( ));
    return std::make_shared<Tile>( 0, rect );
}
