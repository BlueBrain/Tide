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

#include "FolderThumbnailGenerator.h"

#include <QDir>
#include <QPainter>

#include "ThumbnailGeneratorFactory.h"
#include "ThumbnailGenerator.h"
#include "ContentFactory.h"
#include "log.h"

#define FOLDER_THUMBNAIL_COUNT_X  2
#define FOLDER_THUMBNAIL_COUNT_Y  2

FolderThumbnailGenerator::FolderThumbnailGenerator( const QSize& size )
    : ThumbnailGenerator( size )
{
}

QImage FolderThumbnailGenerator::generate( const QString& filename ) const
{
    QDir dir( filename );
    if( dir.exists( ))
        return createFolderImage( dir, true );

    put_flog( LOG_ERROR, "invalid directory: %s",
              filename.toLocal8Bit().constData( ));
    return createErrorImage( "folder" );
}


QImage FolderThumbnailGenerator::generatePlaceholderImage( QDir dir ) const
{
    QImage img = createGradientImage( Qt::black, Qt::white );
    addMetadataToImage( img, dir.path( ));
    return img;
}

QImage FolderThumbnailGenerator::generateUpFolderImage( QDir dir ) const
{
    QImage img = createGradientImage( Qt::darkGray, Qt::lightGray );
    paintText( img, "UP" );
    addMetadataToImage( img, dir.path( ));
    return img;
}

void FolderThumbnailGenerator::addMetadataToImage( QImage& img,
                                                   const QString& url ) const
{
    img.setText( "dir", "true" );
    ThumbnailGenerator::addMetadataToImage( img, url );
}

QVector<QRectF> FolderThumbnailGenerator::calculatePlacement( int nX, int nY, float padding, float totalWidth, float totalHeight) const
{
    const float totalPaddingWidth = padding*(nX+1);
    const float imageWidth = (1.0f-totalPaddingWidth)/(float)nX;

    const float totalPaddingHeight = padding*(nY+1);
    const float imageHeight = (1.0f-totalPaddingHeight)/(float)nY;

    QVector<QRectF> rect;
    for (int j=0; j<nY; j++)
    {
        const float y = padding + j*(imageHeight+padding);
        for (int i=0; i<nX; i++)
        {
            const float x = padding + i*(imageWidth+padding);
            rect.append(QRect(x*totalWidth, y*totalHeight, imageWidth*totalWidth, imageHeight*totalHeight));
        }
    }
    return rect;
}

void FolderThumbnailGenerator::paintThumbnailsMosaic(QImage& img, const QFileInfoList& fileList) const
{
    int numPreviews = std::min(FOLDER_THUMBNAIL_COUNT_X*FOLDER_THUMBNAIL_COUNT_Y, fileList.size());

    if (numPreviews == 0)
        return;

    QVector<QRectF> rect = calculatePlacement(FOLDER_THUMBNAIL_COUNT_X, FOLDER_THUMBNAIL_COUNT_Y, 0.1, img.size().width(), img.size().height());
    QPainter painter( &img );
    for( int i = 0; i < numPreviews; ++i )
    {
        QFileInfo fileInfo = fileList.at( i );
        const QString& filename = fileInfo.absoluteFilePath();

        QImage thumbnail;
        // Avoid recursion into subfolders
        if (QDir(filename).exists())
            thumbnail = createFolderImage(QDir(filename), false);
        else
            thumbnail = ThumbnailGeneratorFactory::getGenerator(filename, size_)->generate(filename);

        painter.drawImage(rect[i], thumbnail);
    }
    painter.end();
}

QImage FolderThumbnailGenerator::createFolderImage(QDir dir, bool generateThumbnails) const
{
    QImage img = generatePlaceholderImage(dir);

    const QFileInfoList& fileList = getSupportedFilesInDir(dir);

    if(generateThumbnails && fileList.size() > 0)
    {
        paintThumbnailsMosaic(img, fileList);
    }
    else
    {
        paintText(img, dir.dirName());
    }

    return img;
}

QFileInfoList FolderThumbnailGenerator::getSupportedFilesInDir(QDir dir) const
{
    dir.setFilter( QDir::Files | QDir::NoDotAndDotDot );
    QStringList filters = ContentFactory::getSupportedFilesFilter();
    filters.append( "*.dcx" );
    dir.setNameFilters( filters );
    return dir.entryInfoList();
}
