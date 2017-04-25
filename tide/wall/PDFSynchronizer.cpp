/*********************************************************************/
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project                  */
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

#include "PDFSynchronizer.h"

#include "Tile.h"
#include "ZoomHelper.h"
#include "scene/ContentWindow.h"
#include "scene/PDFContent.h"

PDFSynchronizer::PDFSynchronizer(std::shared_ptr<PDFTiler> source)
    : LodSynchronizer(source)
    , _source(std::move(source))
{
}

void PDFSynchronizer::update(const ContentWindow& window,
                             const QRectF& visibleArea)
{
    const auto& content = dynamic_cast<const PDFContent&>(*window.getContent());
    const bool pageChanged = content.getPage() != _currentPage;
    _currentPage = content.getPage();

    // Adapted from LODSynchronizer::update to support page change.
    const ZoomHelper helper(window);
    const uint lod = getLod(helper.getContentRect().size().toSize());
    const QSize tilesSurface = getDataSource().getTilesArea(lod);
    const QRectF visibleTilesArea =
        helper.toTilesArea(visibleArea, tilesSurface);

    if (!pageChanged && visibleTilesArea == _visibleTilesArea && lod == _lod)
        return;

    _visibleTilesArea = visibleTilesArea;

    if (pageChanged)
        emit zoomContextTileChanged();

    const bool lodChanged = lod != _lod;
    if (lodChanged)
    {
        _lod = lod;
        emit tilesAreaChanged();
    }

    if (pageChanged || lodChanged)
        emit statisticsChanged();

    setBackgroundTile(_source->getPreviewTileId());

    TiledSynchronizer::updateTiles(getDataSource(), false);
}

QString PDFSynchronizer::getStatistics() const
{
    const auto page = _source->getStatistics();
    return LodSynchronizer::getStatistics() + QString(" ") + page;
}

TilePtr PDFSynchronizer::getZoomContextTile() const
{
    const auto tileId = _source->getPreviewTileId();
    const auto rect = getDataSource().getTileRect(tileId);
    return std::make_shared<Tile>(tileId, rect);
}
