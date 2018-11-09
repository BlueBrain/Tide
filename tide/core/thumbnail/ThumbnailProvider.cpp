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
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#include "ThumbnailProvider.h"

#include "thumbnail.h"

#include <QCache>
#include <QDateTime>
#include <QFileInfo>

#include <cassert>

namespace
{
const int cacheMaxSize = 200;
const QString cacheModificationDateKey{"lastModificationDate"};
const QString folderImg{":/img/folder.png"};
const QString unknownFileImg{":/img/unknownfile.png"};
}

QImage _getPlaceholderImage(const QString& filename)
{
    const auto fileInfo = QFileInfo{filename};
    if (fileInfo.isDir())
    {
        static QImage im{folderImg};
        assert(!im.isNull());
        return im;
    }

    static QImage im{unknownFileImg};
    assert(!im.isNull());
    return im;
}

class ImageCache : public QCache<QString, QImage>
{
public:
    ImageCache() { setMaxCost(cacheMaxSize); }
    bool hasValidImage(const QString& filename) const
    {
        if (!contains(filename))
            return false;

        return QFileInfo{filename}.lastModified().toString() ==
               object(filename)->text(cacheModificationDateKey);
    }

    void insertImage(const QImage& image, const QString& filename)
    {
        // QCache requires a <T>* and takes ownership, a new QImage is required
        auto cacheImage = new QImage{image};
        cacheImage->setText(cacheModificationDateKey,
                            QFileInfo{filename}.lastModified().toString());
        insert(filename, cacheImage);
    }
};

ThumbnailProvider::ThumbnailProvider(const QSize defaultSize)
    : QQuickImageProvider(QQuickImageProvider::Image,
                          QQuickImageProvider::ForceAsynchronousImageLoading)
    , _defaultSize(defaultSize)
    , _cache(new ImageCache)
{
}

QImage ThumbnailProvider::requestImage(const QString& filename, QSize* size,
                                       const QSize& requestedSize)
{
    const QSize newSize{requestedSize.height() > 0 ? requestedSize.height()
                                                   : _defaultSize.height(),
                        requestedSize.width() > 0 ? requestedSize.width()
                                                  : _defaultSize.width()};
    if (size)
        *size = newSize;

    if (_cache->hasValidImage(filename))
        return *_cache->object(filename);

    const auto image = thumbnail::create(filename, newSize);
    if (image.isNull()) // should never happen
        return _getPlaceholderImage(filename);

    _cache->insertImage(image, filename);
    return image;
}

#if TIDE_ASYNC_THUMBNAIL_PROVIDER
#include <QThreadPool>

class AsyncImageResponse : public QQuickImageResponse, public QRunnable
{
public:
    AsyncImageResponse(std::function<QImage()> getImageFunc)
        : _getImageFunc{std::move(getImageFunc)}
    {
        setAutoDelete(false);
    }

    QQuickTextureFactory* textureFactory() const final
    {
        return QQuickTextureFactory::textureFactoryForImage(_image);
    }

    void cancel() final { _canceled = true; }
    void run() final
    {
        if (!_canceled)
            _image = _getImageFunc();
        emit finished();
    }

private:
    std::function<QImage()> _getImageFunc;
    QImage _image;
    bool _canceled = false;
};

AsyncThumbnailProvider::AsyncThumbnailProvider(const QSize defaultSize)
    : _defaultSize{defaultSize}
    , _cache{new ImageCache}
{
}

AsyncThumbnailProvider::~AsyncThumbnailProvider()
{
    QThreadPool::globalInstance()->waitForDone();
}

QQuickImageResponse* AsyncThumbnailProvider::requestImageResponse(
    const QString& filename, const QSize& requestedSize)
{
    const QSize size{requestedSize.height() > 0 ? requestedSize.height()
                                                : _defaultSize.height(),
                     requestedSize.width() > 0 ? requestedSize.width()
                                               : _defaultSize.width()};

    auto response = new AsyncImageResponse([this, filename, size]() {
        {
            const QReadLocker lock(&_mutex);
            if (_cache->hasValidImage(filename))
                return *_cache->object(filename);
        }

        auto image = thumbnail::create(filename, size);
        if (image.isNull()) // should never happen
            return _getPlaceholderImage(filename);

        const QWriteLocker lock(&_mutex);
        _cache->insertImage(image, filename);
        return image;
    });

    QThreadPool::globalInstance()->start(response);
    return response;
}

#endif
