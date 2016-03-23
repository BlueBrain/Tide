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

#include "ImageThumbnailGenerator.h"

#include <QFileInfo>
#include <QImageReader>

#include "log.h"

#define SIZEOF_MEGABYTE  (1024*1024)
#define MAX_IMAGE_FILE_SIZE (100*SIZEOF_MEGABYTE)

ImageThumbnailGenerator::ImageThumbnailGenerator( const QSize& size )
    : ThumbnailGenerator( size )
{
}

QImage ImageThumbnailGenerator::generate( const QString& filename ) const
{
    QImage img;

    QImageReader reader( filename );
    if( reader.canRead( ))
    {
        if (QFileInfo(filename).size() < MAX_IMAGE_FILE_SIZE)
        {
            img = reader.read();
            img = img.scaled(size_, aspectRatioMode_);
        }
        else
        {
            img = createLargeImagePlaceholder();
        }
        addMetadataToImage( img, filename );
        return img;
    }

    put_flog( LOG_ERROR, "could not open image file: '%s'",
              filename.toLatin1().constData( ));
    return createErrorImage( "image" );
}

QImage ImageThumbnailGenerator::createLargeImagePlaceholder() const
{
    QImage img = createGradientImage( Qt::darkBlue, Qt::white );
    paintText( img, "LARGE\nIMAGE" );
    return img;
}
