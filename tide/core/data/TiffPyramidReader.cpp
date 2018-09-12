/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
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

#include "types.h"
#include "utils/log.h"

#include <QPainter>
#include <tiffio.h>

#include <cassert>

namespace
{
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
using TIFFPtr = std::unique_ptr<TIFF, TIFFDeleter>;

QImage::Format _getQImageFormat(const int bytesPerPixel)
{
    switch (bytesPerPixel)
    {
    case 1:
#if QT_VERSION >= 0x050500
        return QImage::Format_Grayscale8;
#else
        return QImage::Format_Indexed8;
#endif
    case 2: // grayscale + alpha => convert to ARGB
        return QImage::Format_ARGB32;
    case 3:
        return QImage::Format_RGB888;
    case 4:
        return QImage::Format_ARGB32;
    default:
        throw std::runtime_error("Unsupported format");
    }
}

void _toARGB32Image(const std::vector<uint16_t>& buffer, QImage& image)
{
    assert(image.format() == QImage::Format_ARGB32);
    assert((size_t)image.byteCount() == 4u * buffer.size());

    auto dst = image.bits();
    for (auto pixel = 0u; pixel < buffer.size(); ++pixel)
    {
        dst[4 * pixel + 0] = buffer[pixel];      // B
        dst[4 * pixel + 1] = buffer[pixel];      // G
        dst[4 * pixel + 2] = buffer[pixel];      // R
        dst[4 * pixel + 3] = buffer[pixel] >> 8; // A
    }
}
}

struct TiffPyramidReader::Impl
{
    TIFFPtr tif;

    Impl(const QString& uri)
        : tif{TIFFOpen(uri.toLocal8Bit().constData(), "r")}
    {
        if (!tif)
            throw std::runtime_error("File could not be opened");

        if (!TIFFIsTiled(tif.get()))
            throw std::runtime_error("Not a tiled tiff image");
    }

    void setDirectory(const uint lod)
    {
        if (!TIFFSetDirectory(tif.get(), lod))
            throw std::runtime_error("Invalid pyramid level");
    }

    void readTile(const QPoint& tileCoord, const int bytesPerPixel,
                  QImage& image)
    {
        if (bytesPerPixel == 2)
            readGrayscaleWithAlphaTile(tileCoord, image);
        else
            readTileData(tileCoord, image.bits());

        if (bytesPerPixel == 4)
            image = image.rgbSwapped(); // Tiff data is stored as ABRG -> ARGB
    }

    void readGrayscaleWithAlphaTile(const QPoint& tileCoord, QImage& image)
    {
        if (!hasAssociatedAlpha())
            throw std::runtime_error("Unknown data layout");

        auto buffer = std::vector<uint16_t>(image.width() * image.height());
        readTileData(tileCoord, buffer.data());
        _toARGB32Image(buffer, image);
    }

    void readTileData(const QPoint& tileCoord, void* buffer)
    {
        validate(tileCoord);
        TIFFReadTile(tif.get(), buffer, tileCoord.x(), tileCoord.y(), 0, 0);
    }

    void validate(const QPoint& tileCoord)
    {
        if (!TIFFCheckTile(tif.get(), tileCoord.x(), tileCoord.y(), 0, 0))
            throw std::runtime_error("Invalid coordinates");
    }

    bool hasAssociatedAlpha()
    {
        int extra = EXTRASAMPLE_UNSPECIFIED;
        TIFFGetField(tif.get(), TIFFTAG_EXTRASAMPLES, &extra);
        return extra == EXTRASAMPLE_ASSOCALPHA;
    }
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

QImage TiffPyramidReader::readTile(const int i, const int j, const uint lod)
{
    try
    {
        _impl->setDirectory(lod);

        const auto bytesPerPixel = getBytesPerPixel();
        const auto format = _getQImageFormat(bytesPerPixel);

        const auto tileSize = getTileSize();
        const auto tileCoord =
            QPoint{i * tileSize.width(), j * tileSize.height()};

        auto image = QImage{tileSize, format};
        _impl->readTile(tileCoord, bytesPerPixel, image);
        return image;
    }
    catch (const std::runtime_error& e)
    {
        print_log(LOG_WARN, LOG_TIFF, "%s for tile (%d, %d) @ LOD %d", e.what(),
                  i, j, lod);
        return QImage();
    }
}

QImage TiffPyramidReader::readTopLevelImage()
{
    auto image = readTile(0, 0, findTopPyramidLevel());
    const auto croppedSize = getImageSize(); // assume directory is unchanged
    if (image.size() != croppedSize)
        image = image.copy(QRect(QPoint(), croppedSize));
    return image;
}

QSize TiffPyramidReader::readSize(const uint lod)
{
    try
    {
        _impl->setDirectory(lod);
        return getImageSize();
    }
    catch (const std::runtime_error&)
    {
        print_log(LOG_WARN, LOG_TIFF, "Invalid pyramid level: %d", lod);
        return QSize();
    }
}

QImage TiffPyramidReader::readImage(const uint lod)
{
    try
    {
        _impl->setDirectory(lod);

        const auto bytesPerPixel = getBytesPerPixel();
        const auto format = _getQImageFormat(bytesPerPixel);

        auto tile = QImage{getTileSize(), format};
        auto image = QImage{getImageSize(), format};

        QPainter painter{&image};
        for (int y = 0; y < image.height(); y += tile.height())
        {
            for (int x = 0; x < image.width(); x += tile.width())
            {
                _impl->readTile({x, y}, bytesPerPixel, tile);
                painter.drawImage(x, y, tile);
            }
        }
        return image;
    }
    catch (const std::runtime_error& e)
    {
        print_log(LOG_WARN, LOG_TIFF, "%s for image LOD %d", e.what(), lod);
        return QImage();
    }
}
