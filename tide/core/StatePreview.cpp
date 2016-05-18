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

#include "StatePreview.h"

#include <QRectF>

#include "ContentWindow.h"
#include "log.h"

#include "thumbnail/ThumbnailGeneratorFactory.h"
#include "thumbnail/ThumbnailGenerator.h"

#include <QFileInfo>
#include <QPainter>
#include <QImageReader>

#include <boost/foreach.hpp>

namespace
{
static const QSize PREVIEW_IMAGE_SIZE( 512, 512 );
static const QSize THUMBNAIL_SIZE( 128, 128 );
}

StatePreview::StatePreview( const QString &dcxFileName )
    : dcxFileName_( dcxFileName )
{
}

QString StatePreview::getFileExtension()
{
    return QString( ".dcxpreview" );
}

QImage StatePreview::getImage() const
{
    return previewImage_;
}

QString StatePreview::previewFilename() const
{
    QFileInfo fileinfo( dcxFileName_ );

    const QString extension = fileinfo.suffix().toLower();
    if( extension != "dcx" )
    {
        put_flog( LOG_WARN, "wrong state file extension: '%s' for session: '%s'"
                            "(expected: .dcx)",
                  extension.toLocal8Bit().constData( ),
                  dcxFileName_.toLocal8Bit().constData( ));
        return QString();
    }
    return fileinfo.path() + "/" + fileinfo.completeBaseName() + getFileExtension();
}

void StatePreview::generateImage( const QSize& wallDimensions,
                                  const ContentWindowPtrs &contentWindows )
{
    QSize previewDimension( wallDimensions );
    previewDimension.scale( PREVIEW_IMAGE_SIZE, Qt::KeepAspectRatio );

    // Transparent image
    QImage preview( previewDimension, QImage::Format_ARGB32 );
    preview.fill( qRgba( 0, 0, 0, 0 ));

    QPainter painter( &preview );

    // Paint all Contents at their correct location
    BOOST_FOREACH( ContentWindowPtr window, contentWindows )
    {
        if( window->getContent()->getType() == CONTENT_TYPE_PIXEL_STREAM ||
            window->getContent()->getType() == CONTENT_TYPE_WEBBROWSER )
            continue;

        const QRectF& winCoord = window->getCoordinates();
        const qreal ratio = (qreal)previewDimension.width() /
                            (qreal)wallDimensions.width();
        const QRectF area( winCoord.topLeft() * ratio,
                           winCoord.size() * ratio );

        const QString& filename = window->getContent()->getURI();
        ThumbnailGeneratorPtr generator =
            ThumbnailGeneratorFactory::getGenerator( filename, THUMBNAIL_SIZE );
        const QImage image = generator->generate( filename );

        painter.drawImage( area, image );
    }

    painter.end();

    previewImage_ = preview;
}

bool StatePreview::saveToFile() const
{
    const bool success = previewImage_.save( previewFilename(), "PNG" );

    if( !success )
        put_flog( LOG_ERROR, "Saving StatePreview image failed: '%s'",
                  previewFilename().toLocal8Bit().constData( ));

    return success;
}

bool StatePreview::loadFromFile()
{
    QImageReader reader( previewFilename( ));
    if ( reader.canRead( ))
    {
        return reader.read( &previewImage_ );
    }
    return false;
}
