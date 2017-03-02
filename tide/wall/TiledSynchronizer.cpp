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

#include "TiledSynchronizer.h"

#include "DataSource.h"
#include "Tile.h"
#include "network/WallToWallChannel.h"

TiledSynchronizer::TiledSynchronizer( const TileSwapPolicy policy )
    : _lod( 0 )
    , _policy( policy )
    , _syncSwapPending( false )
{}

void TiledSynchronizer::onSwapReady( TilePtr tile )
{
    if( _policy == SwapTilesSynchronously &&
            _syncSet.find( tile->getId( )) != _syncSet.end( ))
    {
        _tilesReadyToSwap.insert( tile );
        _tilesReadySet.insert( tile->getId( ));
    }
    else
        tile->swapImage();
}

void TiledSynchronizer::updateTiles( const DataSource& source,
                                     const bool updateExistingTiles )
{
    Indices visibleSet = source.computeVisibleSet( _visibleTilesArea, _lod );
    visibleSet = set_difference( visibleSet, _ignoreSet );

    const Indices addedTiles = set_difference( visibleSet, _visibleSet );
    const Indices removedTiles = set_difference( _visibleSet, visibleSet );

    for( auto i : addedTiles )
        emit addTile( std::make_shared<Tile>( i, source.getTileRect( i ),
                                              source.getTileFormat( i )));

    if( updateExistingTiles )
    {
        const Indices currentTiles = set_difference( _visibleSet, removedTiles);
        for( auto i : currentTiles )
            emit updateTile( i, source.getTileRect( i ),
                             source.getTileFormat( i ));
    }

    if( _policy == SwapTilesSynchronously )
    {
        if( updateExistingTiles )
        {
            _syncSet = visibleSet;
            _syncSwapPending = true;
        }
        else
            _syncSet = set_difference( _syncSet, removedTiles );
    }

    for( auto i : removedTiles )
        _removeTile( i );
    _removeLaterSet = set_difference( _removeLaterSet, addedTiles );

    _visibleSet = visibleSet;
}

bool TiledSynchronizer::swapTiles( WallToWallChannel& channel )
{
    if( !_syncSwapPending )
        return false;

    const bool swap = set_difference( _syncSet, _tilesReadySet ).empty();
    if( !channel.allReady( swap ))
        return false;

    for( auto i : _removeLaterSet )
        emit removeTile( i );
    _removeLaterSet.clear();

    for( auto& tile : _tilesReadyToSwap )
        tile->swapImage();
    _tilesReadyToSwap.clear();
    _tilesReadySet.clear();
    _syncSet.clear();

    _syncSwapPending = false;
    return true;
}

void TiledSynchronizer::_removeTile( const size_t tileIndex )
{
    if( _policy == SwapTilesSynchronously && _syncSwapPending )
        _removeLaterSet.insert( tileIndex );
    else
        emit removeTile( tileIndex );
}
