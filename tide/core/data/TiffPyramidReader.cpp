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

#include "TiffPyramidReader.h"

#include "log.h"
#include "types.h"

#include <QPainter>
#include <tiffio.h>

struct TiffStaticInit
{
    TiffStaticInit()
    {
        TIFFSetWarningHandler(tiffMessageLoggerWarn);
        TIFFSetErrorHandler(tiffMessageLoggerErr);
    }
};
static TiffStaticInit instance;

struct TIFFDeleter
{
    void operator()(TIFF* file) { TIFFClose(file); }
};
typedef std::unique_ptr<TIFF, TIFFDeleter> TIFFPtr;

struct TiffPyramidReader::Impl
{
    Impl(const QString& uri)
        : tif{TIFFOpen(uri.toLocal8Bit().constData(), "r")}
    {
        if (!tif)
            throw std::runtime_error("File could not be opened");

        if (!TIFFIsTiled(tif.get()))
            throw std::runtime_error("Not a tiled tiff image");
    }
    TIFFPtr tif;
};

TiffPyramidReader::TiffPyramidReader(const QString& uri)
    : _impl{new Impl{uri}}
{
}

TiffPyramidReader::~TiffPyramidReader()
{
}

QSize TiffPyramidReader::getImageSize() const
{
    QSize size;
    TIFFGetField(_impl->tif.get(), TIFFTAG_IMAGEWIDTH, &size.rwidth());
    TIFFGetField(_impl->tif.get(), TIFFTAG_IMAGELENGTH, &size.rheight());
    return size;
}

QSize TiffPyramidReader::getTileSize() const
{
    QSize size;
    TIFFGetField(_impl->tif.get(), TIFFTAG_TILEWIDTH, &size.rwidth());
    TIFFGetField(_impl->tif.get(), TIFFTAG_TILELENGTH, &size.rheight());
    return size;
}

int TiffPyramidReader::getBytesPerPixel() const
{
    int value = 0;
    TIFFGetField(_impl->tif.get(), TIFFTAG_SAMPLESPERPIXEL, &value);
    return value;
}

uint TiffPyramidReader::findTopPyramidLevel()
{
    return findLevel(getTileSize());
}

uint TiffPyramidReader::findLevel(const QSize& imageSize)
{
    TIFFSetDirectory(_impl->tif.get(), 0);

    uint level = 0;
    while (getImageSize() > imageSize && TIFFReadDirectory(_impl->tif.get()))
        ++level;

    return level;
}

QImage::Format _getImageFormat(const int bytesPerPixel)
{
    switch (bytesPerPixel)
    {
    case 1:
#if QT_VERSION >= 0x050500
        return QImage::Format_Grayscale8;
#else
        return QImage::Format_Indexed8;
#endif
    case 3:
        return QImage::Format_RGB888;
    case 4:
        return QImage::Format_ARGB32;
    default:
        return QImage::Format_Invalid;
    }
}

QImage TiffPyramidReader::readTile(const int i, const int j, const uint lod)
{
    const QSize tileSize = getTileSize();

    if (!TIFFSetDirectory(_impl->tif.get(), lod))
    {
        print_log(LOG_WARN, LOG_TIFF, "Invalid pyramid level: %d", lod);
        return QImage();
    }

    const QPoint tile(i * tileSize.width(), j * tileSize.height());
    if (!TIFFCheckTile(_impl->tif.get(), tile.x(), tile.y(), 0, 0))
    {
        print_log(LOG_WARN, LOG_TIFF, "Invalid tile (%d, %d) @ LOD %d", i, j,
                  lod);
        return QImage();
    }

    QImage image(tileSize, _getImageFormat(getBytesPerPixel()));
    TIFFReadTile(_impl->tif.get(), image.bits(), tile.x(), tile.y(), 0, 0);
    return image;
}

QImage TiffPyramidReader::readTopLevelImage()
{
    QImage image = readTile(0, 0, findTopPyramidLevel());
    const QSize croppedSize = getImageSize(); // assume directory is unchanged
    if (image.size() != croppedSize)
        image = image.copy(QRect(QPoint(), croppedSize));
    return image;
}

QSize TiffPyramidReader::readSize(const uint lod)
{
    if (!TIFFSetDirectory(_impl->tif.get(), lod))
    {
        print_log(LOG_WARN, LOG_TIFF, "Invalid pyramid level: %d", lod);
        return QSize();
    }
    return getImageSize();
}

QImage TiffPyramidReader::readImage(const uint lod)
{
    if (!TIFFSetDirectory(_impl->tif.get(), lod))
    {
        print_log(LOG_WARN, LOG_TIFF, "Invalid pyramid level: %d", lod);
        return QImage();
    }

    const auto format = _getImageFormat(getBytesPerPixel());
    QImage tile{getTileSize(), format};
    QImage image{getImageSize(), format};
    QPainter painter{&image};

    for (int y = 0; y < image.height(); y += tile.height())
    {
        for (int x = 0; x < image.width(); x += tile.width())
        {
            TIFFReadTile(_impl->tif.get(), tile.bits(), x, y, 0, 0);
            painter.drawImage(x, y, tile);
        }
    }
    return image;
}
