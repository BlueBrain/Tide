/*********************************************************************/
/* Copyright (c) 2015, EPFL/Blue Brain Project                       */
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

#include "PixelStreamSynchronizer.h"

#include "network/WallToWallChannel.h"
#include "PixelStreamUpdater.h"
#include "scene/ContentWindow.h"
#include "Tile.h"
#include "ZoomHelper.h"

PixelStreamSynchronizer::PixelStreamSynchronizer()
    : TiledSynchronizer( TileSwapPolicy::SwapTilesSynchronously )
    , _tilesDirty( true )
    , _updateExistingTiles( false )
{}

void
PixelStreamSynchronizer::setDataSource( PixelStreamUpdaterSharedPtr updater )
{
    if( _updater )
        _updater->disconnect( this );

    _updater = updater;

    connect( _updater.get(), &PixelStreamUpdater::pictureUpdated,
             this, &PixelStreamSynchronizer::_onPictureUpdated );
}

void PixelStreamSynchronizer::update( const ContentWindow& window,
                                      const QRectF& visibleArea )
{
    // Tiles area corresponds to Content dimensions for PixelStreams
    const QSize tilesSurface = window.getContent()->getDimensions();

    ZoomHelper helper( window );
    const QRectF visibleTilesArea = helper.toTilesArea( visibleArea,
                                                        tilesSurface );

    if( _visibleTilesArea == visibleTilesArea )
        return;

    _visibleTilesArea = visibleTilesArea;
    _tilesDirty = true;
}

void PixelStreamSynchronizer::synchronize( WallToWallChannel& channel )
{
    if( !_updater )
        return;

    if( TiledSynchronizer::swapTiles( channel ))
    {
        _fpsCounter.tick();
        emit statisticsChanged();

        _tilesArea = _updater->getTilesArea( 0 );
        emit tilesAreaChanged();

        _updater->getNextFrame();
    }

    _updater->synchronizeFramesSwap( channel );

    if( _tilesDirty )
    {
        TiledSynchronizer::updateTiles( *_updater, _updateExistingTiles );
        _tilesDirty = false;
        _updateExistingTiles = false;
    }
}

QSize PixelStreamSynchronizer::getTilesArea() const
{
    return _tilesArea;
}

QString PixelStreamSynchronizer::getStatistics() const
{
    return _fpsCounter.toString() + " fps";
}

ImagePtr PixelStreamSynchronizer::getTileImage( const uint tileIndex ) const
{
    if( !_updater )
        return ImagePtr();

    return _updater->getTileImage( tileIndex );
}

void PixelStreamSynchronizer::_onPictureUpdated()
{
    _tilesDirty = true;
    _updateExistingTiles = true;
}
