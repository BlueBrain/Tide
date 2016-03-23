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

#include "AsyncImageLoader.h"

#include "thumbnail/ThumbnailGeneratorFactory.h"
#include "thumbnail/ThumbnailGenerator.h"

#include <QFileInfo>
#include <QDateTime>

#define USE_CACHE
#define CACHE_MAX_SIZE 100
#define MODIFICATION_DATE_KEY "lastModificationDate"

AsyncImageLoader::AsyncImageLoader( const QSize& defaultSize )
    : _defaultSize( defaultSize )
{
    _cache.setMaxCost( CACHE_MAX_SIZE );
}

void AsyncImageLoader::loadImage( const QString& filename, const int index )
{
#ifdef USE_CACHE
    if( imageInCache( filename ))
    {
        emit imageLoaded( index, *_cache[filename] );
        return;
    }
#endif

    QImage image = ThumbnailGeneratorFactory::getGenerator( filename, _defaultSize )->generate( filename );
    if( !image.isNull( ))
    {
#ifdef USE_CACHE
        // QCache requires a <T>* and takes ownership, so we have to create new QImage
        QImage* cacheImage = new QImage( image );
        cacheImage->setText( MODIFICATION_DATE_KEY,
                             QFileInfo( filename ).lastModified().toString( ));
        _cache.insert( filename, cacheImage );
#endif
        emit imageLoaded( index, image );
    }

    emit imageLoadingFinished();
}

bool AsyncImageLoader::imageInCache( const QString& filename ) const
{
    if( !_cache.contains( filename ))
        return false;

    const QFileInfo info( filename );
    return info.lastModified().toString() ==
            _cache.object( filename )->text( MODIFICATION_DATE_KEY );
}
