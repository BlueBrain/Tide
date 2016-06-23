/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
/*                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>         */
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

#include "ThumbnailProvider.h"

#include "ThumbnailGenerator.h"
#include "ThumbnailGeneratorFactory.h"

#include <QDateTime>
#include <QFileInfo>
#include <QImageReader>

#include <cassert>

namespace
{
const int cacheMaxSize = 200;
const QString cacheModificationDateKey( "lastModificationDate" );
const char* folderImg = "qrc:/img/folder.png";
const char* unknownFileImg = "qrc:/img/unknownfile.png";
}

ThumbnailProvider::ThumbnailProvider( const QSize defaultSize )
    : QQuickImageProvider( QQuickImageProvider::Image,
                           QQuickImageProvider::ForceAsynchronousImageLoading )
    , _defaultSize( defaultSize )
{
    _cache.setMaxCost( cacheMaxSize );
}

QImage ThumbnailProvider::requestImage( const QString& filename, QSize* size,
                                        const QSize& requestedSize )
{
    const QSize newSize( requestedSize.height() > 0 ?
                             requestedSize.height() : _defaultSize.height(),
                         requestedSize.width() > 0 ?
                             requestedSize.width() : _defaultSize.width( ));
    if( size )
        *size = newSize;

    if( _isImageInCache( filename ))
        return *_cache[filename];

    auto generator = ThumbnailGeneratorFactory::getGenerator( filename,
                                                              newSize );
    const QImage image = generator->generate( filename );
    if( !image.isNull( ))
    {
        // QCache requires a <T>* and takes ownership, a new QImage is required
        QImage* cacheImage = new QImage( image );
        cacheImage->setText( cacheModificationDateKey,
                             QFileInfo( filename ).lastModified().toString( ));
        _cache.insert( filename, cacheImage );
        return image;
    }

    // Thumbnail generation failed, return a placeholder
    const QFileInfo fileInfo( filename );
    if( fileInfo.isFile( ))
    {
        static QImage im( unknownFileImg );
        assert( !im.isNull( ));
        return im;
    }
    if( fileInfo.isDir( ))
    {
        static QImage im( folderImg );
        assert( !im.isNull( ));
        return im;
    }

    return image; // Silence compiler warning
}

bool ThumbnailProvider::_isImageInCache( const QString& filename ) const
{
    if( !_cache.contains( filename ))
        return false;

    const QFileInfo info( filename );
    return info.lastModified().toString() ==
            _cache.object( filename )->text( cacheModificationDateKey );
}
