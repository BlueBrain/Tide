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

#include "ImagePyramidDataSource.h"

#include "data/TiffPyramidReader.h"
#include "tools/LodTools.h"
#include "utils/log.h"

namespace
{
const QSize previewSize{1920, 1920};

uint _getTileSize(const TiffPyramidReader& tif)
{
    const auto tileSize = tif.getTileSize();
    if (tileSize.width() != tileSize.height())
        throw std::runtime_error("Non-square tiles are not supported");
    return tileSize.width();
}
}

ImagePyramidDataSource::ImagePyramidDataSource(const QString& uri)
    : _uri{uri}
{
    try
    {
        TiffPyramidReader tif{uri};
        _lodTool =
            std::make_unique<LodTools>(tif.getImageSize(), _getTileSize(tif));
        _previewImageSize = tif.readSize(tif.findLevel(previewSize));
    }
    catch (const std::runtime_error& e)
    {
        _lodTool = std::make_unique<LodTools>(QSize(), 1);
        _valid = false;
        print_log(LOG_WARN, LOG_CONTENT, "Failed opening TIFF file: %s - %s",
                  uri.toLocal8Bit().constData(), e.what());
    }
}

ImagePyramidDataSource::~ImagePyramidDataSource()
{
}

QRect ImagePyramidDataSource::getTileRect(const uint tileId) const
{
    if (tileId == getPreviewTileId())
        return {QPoint(), _previewImageSize};

    return LodTiler::getTileRect(tileId);
}

uint ImagePyramidDataSource::getPreviewTileId() const
{
    return std::numeric_limits<uint>::max();
}

QImage ImagePyramidDataSource::getCachableTileImage(
    const uint tileId, const deflect::View view) const
{
    Q_UNUSED(view);

    if (!_valid)
        return QImage();

    TiffPyramidReader tif{_uri};

    QImage image;
    if (tileId == getPreviewTileId())
        image = tif.readImage(tif.findLevel(previewSize));
    else
    {
        const auto index = _getLodTool().getTileIndex(tileId);
        image = tif.readTile(index.x, index.y, index.lod);
    }

    // TIFF tiles all have a fixed size. Those at the top of the pyramid
    // (or at the borders) are padded, but Tide expects to get tiles of the
    // exact dimensions (not padding) for uploading as GL textures.
    const auto expectedSize = getTileRect(tileId).size();
    if (image.size() != expectedSize)
        image = image.copy(QRect(QPoint(), expectedSize));
    return image;
}
