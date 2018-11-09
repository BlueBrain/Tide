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

#include "LodSynchronizer.h"

#include "datasources/DataSource.h"
#include "qml/Tile.h"
#include "scene/Window.h"
#include "scene/ZoomHelper.h"

#include <QTextStream>

LodSynchronizer::LodSynchronizer(DataSourceSharedPtr source)
    : TiledSynchronizer{TileSwapPolicy::SwapTilesIndependently}
    , _source{std::move(source)}
{
    _source->synchronizers.register_(this);

    connect(this, &ContentSynchronizer::zoomContextVisibleChanged,
            [this] { _zoomContextTileDirty = true; });
}

LodSynchronizer::~LodSynchronizer()
{
    _source->synchronizers.deregister(this);
}

void LodSynchronizer::update(const Window& window, const QRectF& visibleArea)
{
    update(window, visibleArea, false);
}

void LodSynchronizer::updateTiles()
{
    TiledSynchronizer::updateTiles();

    if (_zoomContextTileDirty)
    {
        _zoomContextTileDirty = false;
        emit zoomContextTileChanged(getZoomContextVisible());
    }
}

QSize LodSynchronizer::_getTilesArea(const uint lod) const
{
    return getDataSource().getTilesArea(lod, getChannel());
}

QString LodSynchronizer::getStatistics() const
{
    auto stats = QString();
    QTextStream stream{&stats};
    stream << "LOD:  " << getLod() << "/" << getLodCount() - 1;
    const auto& area = _getTilesArea(getLod());
    stream << "  res: " << area.width() << "x" << area.height();
    return stats;
}

TilePtr LodSynchronizer::createZoomContextTile() const
{
    const auto id = getDataSource().getPreviewTileId();
    return Tile::create(id, getDataSource().getTileRect(id));
}

uint LodSynchronizer::getLod() const
{
    return _lod;
}

uint LodSynchronizer::getLodCount() const
{
    return getDataSource().getMaxLod() + 1;
}

void LodSynchronizer::update(const Window& window, const QRectF& visibleArea,
                             const bool forceUpdate)
{
    const auto lod = _findCurrentLod(window);
    const auto tilesArea = _computeVisibleTilesArea(window, visibleArea, lod);

    if (!forceUpdate && lod == _lod && tilesArea == _visibleTilesArea[lod])
        return;

    _updateVisibleTileAreas(window, visibleArea);
    _updateLod(lod);

    markTilesDirty();

    if (forceUpdate)
        _zoomContextTileDirty = true;
}

const DataSource& LodSynchronizer::getDataSource() const
{
    return *_source;
}

QRectF LodSynchronizer::getVisibleTilesArea(const uint lod) const
{
    return _visibleTilesArea.at(lod);
}

void LodSynchronizer::_updateLod(const uint lod)
{
    if (lod == _lod)
        return;

    _lod = lod;
    emit lodChanged(_lod);
    emit statisticsChanged();
}

void LodSynchronizer::_updateVisibleTileAreas(const Window& window,
                                              const QRectF& visibleArea)
{
    _visibleTilesArea.resize(getLodCount());

    for (auto lod = 0u; lod < getLodCount(); ++lod)
    {
        _visibleTilesArea[lod] =
            _computeVisibleTilesArea(window, visibleArea, lod);
    }
}

QRectF LodSynchronizer::_computeVisibleTilesArea(const Window& window,
                                                 const QRectF& visibleArea,
                                                 const uint lod) const
{
    const auto tilesSurface = getDataSource().getTilesArea(lod, getChannel());
    return ZoomHelper{window}.toTilesArea(visibleArea, tilesSurface);
}

uint LodSynchronizer::_findCurrentLod(const Window& window) const
{
    return _findLod(ZoomHelper{window}.getContentRect().size().toSize());
}

uint LodSynchronizer::_findLod(const QSize& targetDisplaySize) const
{
    auto lod = 0u;
    auto nextLOD = getDataSource().getTilesArea(1, 0);
    const auto maxLod = getDataSource().getMaxLod();
    while (targetDisplaySize.width() < nextLOD.width() &&
           targetDisplaySize.height() < nextLOD.height() && lod < maxLod)
    {
        nextLOD = getDataSource().getTilesArea(++lod + 1, 0);
    }
    return lod;
}
