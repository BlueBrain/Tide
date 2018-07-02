/*********************************************************************/
/* Copyright (c) 2015-2017, EPFL/Blue Brain Project                  */
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

#include "MovieSynchronizer.h"

#include "MovieUpdater.h"
#include "Tile.h"
#include "ZoomHelper.h"
#include "scene/Window.h"

MovieSynchronizer::MovieSynchronizer(std::shared_ptr<MovieUpdater> updater,
                                     const deflect::View view)
    : TiledSynchronizer{TileSwapPolicy::SwapTilesSynchronously}
    , _updater{std::move(updater)}
    , _view{view}
{
    _updater->synchronizers.insert(this);

    connect(_updater.get(), &MovieUpdater::pictureUpdated, this,
            &MovieSynchronizer::_onPictureUpdated);
}

MovieSynchronizer::~MovieSynchronizer()
{
    _updater->synchronizers.erase(this);
}

void MovieSynchronizer::update(const Window& window, const QRectF& visibleArea)
{
    if (_updater->isSkipping())
        emit sliderPositionChanged();

    // Tiles area corresponds to Content dimensions for Movies
    const auto tilesSurface = window.getContent().getDimensions();

    const auto visibleTilesArea =
        ZoomHelper{window}.toTilesArea(visibleArea, tilesSurface);

    if (_visibleTilesArea == visibleTilesArea)
        return;

    _visibleTilesArea = visibleTilesArea;
    _tilesDirty = true;
}

void MovieSynchronizer::updateTiles()
{
    if (_tilesDirty)
    {
        TiledSynchronizer::updateTiles();
        _tilesDirty = false;
        _updateExistingTiles = false;
    }
}

void MovieSynchronizer::swapTiles()
{
    TiledSynchronizer::swapTiles();

    if (!_updater->isPaused() || _updater->isSkipping())
    {
        _fpsCounter.tick();
        emit statisticsChanged();
        emit sliderPositionChanged();
    }
}

QSize MovieSynchronizer::getTilesArea() const
{
    return getDataSource().getTilesArea(0, 0);
}

QString MovieSynchronizer::getStatistics() const
{
    return QString("%1 / %2").arg(_fpsCounter.toString(),
                                  _updater->getStatistics());
}

deflect::View MovieSynchronizer::getView() const
{
    return _view;
}

const DataSource& MovieSynchronizer::getDataSource() const
{
    return *_updater;
}

bool MovieSynchronizer::hasVisibleTiles() const
{
    return !_visibleTilesArea.isEmpty();
}

qreal MovieSynchronizer::getSliderPosition() const
{
    // The slider follows user input while skipping to keep things smooth
    return _updater->isSkipping() ? _updater->getSkipPosition()
                                  : _updater->getPosition();
}

void MovieSynchronizer::_onPictureUpdated()
{
    _updateExistingTiles = true;
    _tilesDirty = true;
}
