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

#ifndef TIFFPYRAMIDREADER_H
#define TIFFPYRAMIDREADER_H

#include <QImage>
#include <memory>

/**
 * Reader for TIFF image pyramid files.
 *
 * TIFF image pyramids start at level 0 (full resolution). The dimensions of
 * each level above has half the dimensions of the level beneath it.
 * The top of the pyramid is a single image, whose dimensions are equal to (or
 * directly smaller than) the size of a single tile.
 *
 * Example pyramid for an image of size (512x512) with tile size (128x128):
 * level 2   /-\     <-- top    (128x128),  1 tile
 * level 1  /---\               (256x256),  4 tiles
 * level 0 /-----\   <-- base   (512x512), 16 tiles
 */
class TiffPyramidReader
{
public:
    /**
     * Open an image file for reading.
     * @param uri the TIFF image file to open
     * @throw std::runtime_error if the file is not a supported image pyramid
     */
    TiffPyramidReader(const QString& uri);

    /** Close the image. */
    ~TiffPyramidReader();

    /** Get the full size of the image. */
    QSize getImageSize() const;

    /** Get the size of the image tiles (all tiles have the same size). */
    QSize getTileSize() const;

    /** Get the number of bytes by pixel. */
    int getBytesPerPixel() const;

    /** @return true if the image has an alpha channel. */
    bool hasAlphaChannel() const;

    /** Find the index of the top level of the pyramid. */
    uint findTopPyramidLevel();

    /**
     * Find the level of the pyramid whose dimensions are equal to or directly
     * smaller than the target image size.
     */
    uint findLevel(const QSize& imageSize);

    /**
     * Find the level of the pyramid whose dimensions are equal to or directly
     * larger than the target image size.
     */
    uint findLevelForImageOfMin(const QSize& imageSize);

    /** Read a tile at the given indices and level of detail. */
    QImage readTile(int i, int j, uint lod);

    /** Read the image at the top of the pyramid (i.e. <= tileSize). */
    QImage readTopLevelImage();

    /** Get the size of the pyramid at a certain level of detail. */
    QSize readSize(uint lod);

    /** Read an entire level of the pyramid as a single image. */
    QImage readImage(uint lod);

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

#endif
