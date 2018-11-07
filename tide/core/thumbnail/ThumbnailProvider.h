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

#ifndef THUMBNAILPROVIDER_H
#define THUMBNAILPROVIDER_H

#include <QtQuick/QQuickImageProvider>
#include <memory>

class ImageCache;

/**
 * Provide thumbnails for files and folders to the Qml FileBrowser.
 *
 * The provider maintains an internal cache to speed up the request of
 * previously generated images. It also takes care of regenerating the thumbnail
 * if the file has been modified since the last request (checking the file
 * modification date).
 *
 * Example of correct usage in Qml:
 * Image {
 *     source: "image://thumbnail/" + filePath
 *     cache: false
 * }
 */
class ThumbnailProvider : public QQuickImageProvider
{
public:
    ThumbnailProvider(const QSize defaultSize = QSize(512, 512));

    QImage requestImage(const QString& filename, QSize* size,
                        const QSize& requestedSize) final;

private:
    const QSize _defaultSize;
    std::unique_ptr<ImageCache> _cache;
};

// Need at least Qt 5.6.3 or 5.7.1 because of the following segfault bug:
// https://bugreports.qt.io/browse/QTBUG-56056
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 3)) && \
    (QT_VERSION != QT_VERSION_CHECK(5, 7, 0))
#define TIDE_ASYNC_THUMBNAIL_PROVIDER 1
#include <QReadWriteLock>

/**
 * Provide thumbnails for files and folders to the Qml FileBrowser.
 *
 * This is a multi-threaded version of the ThumbnailProvider.
 */
class AsyncThumbnailProvider : public QQuickAsyncImageProvider
{
public:
    AsyncThumbnailProvider(const QSize defaultSize = QSize(512, 512));
    ~AsyncThumbnailProvider();

    QQuickImageResponse* requestImageResponse(const QString& filename,
                                              const QSize& size) final;

private:
    const QSize _defaultSize;
    std::unique_ptr<ImageCache> _cache;
    QReadWriteLock _mutex;
};

#endif

#endif
