/*********************************************************************/
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
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

#include "log.h"
#include "scene/ContentFactory.h"
#include "thumbnail.h"

#include <QDir>
#include <QPainter>

namespace
{
const int FOLDER_THUMBNAILS_X = 2;
const int FOLDER_THUMBNAILS_Y = 2;
const float FOLDER_THUMBNAILS_PADDING = 0.1;
const QString FOLDER_TEXT("folder");
}

FolderThumbnailGenerator::FolderThumbnailGenerator(const QSize& size)
    : ThumbnailGenerator(size)
{
}

QImage FolderThumbnailGenerator::generate(const QString& filename) const
{
    const QDir dir(filename);
    if (dir.exists())
        return _createFolderImage(dir, true);

    print_log(LOG_ERROR, LOG_CONTENT, "invalid directory: %s",
              filename.toLocal8Bit().constData());
    return createErrorImage("folder");
}

QImage FolderThumbnailGenerator::_createFolderImage(
    const QDir& dir, const bool generateThumbnails) const
{
    QImage img = createGradientImage(Qt::black, Qt::white);

    const QFileInfoList& fileList = _getSupportedFilesInDir(dir);

    if (generateThumbnails && fileList.size() > 0)
        _paintThumbnailsMosaic(img, fileList);
    else
        paintText(img, FOLDER_TEXT);

    return img;
}

void FolderThumbnailGenerator::_paintThumbnailsMosaic(
    QImage& img, const QFileInfoList& fileList) const
{
    const int numPreviews =
        std::min(FOLDER_THUMBNAILS_X * FOLDER_THUMBNAILS_Y, fileList.size());
    if (numPreviews == 0)
        return;

    QVector<QRectF> rect =
        _calculatePlacement(FOLDER_THUMBNAILS_X, FOLDER_THUMBNAILS_Y,
                            FOLDER_THUMBNAILS_PADDING, img.size().width(),
                            img.size().height());
    QPainter painter(&img);
    for (int i = 0; i < numPreviews; ++i)
    {
        const auto filename = fileList.at(i).absoluteFilePath();

        QImage thumbnail;
        // Avoid recursion into subfolders
        if (QDir(filename).exists())
            thumbnail = _createFolderImage(QDir(filename), false);
        else
            thumbnail = thumbnail::create(filename, rect[i].size().toSize());

        // Draw the thumbnail centered in its rectangle, preserving aspect ratio
        QSizeF paintedSize(thumbnail.size());
        paintedSize.scale(rect[i].size(), Qt::KeepAspectRatio);
        QRectF paintRect(QPointF(), paintedSize);
        paintRect.moveCenter(rect[i].center());
        painter.drawImage(paintRect, thumbnail);
    }
    painter.end();
}

QVector<QRectF> FolderThumbnailGenerator::_calculatePlacement(
    int nX, int nY, float padding, float totalWidth, float totalHeight) const
{
    const float totalPaddingWidth = padding * (nX + 1);
    const float imageWidth = (1.0f - totalPaddingWidth) / (float)nX;

    const float totalPaddingHeight = padding * (nY + 1);
    const float imageHeight = (1.0f - totalPaddingHeight) / (float)nY;

    QVector<QRectF> rect;
    for (int j = 0; j < nY; ++j)
    {
        const float y = padding + j * (imageHeight + padding);
        for (int i = 0; i < nX; ++i)
        {
            const float x = padding + i * (imageWidth + padding);
            rect.append(QRect(x * totalWidth, y * totalHeight,
                              imageWidth * totalWidth,
                              imageHeight * totalHeight));
        }
    }
    return rect;
}

QFileInfoList FolderThumbnailGenerator::_getSupportedFilesInDir(QDir dir) const
{
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QStringList filters = ContentFactory::getSupportedFilesFilter();
    filters.append("*.dcx");
    dir.setNameFilters(filters);
    return dir.entryInfoList();
}
