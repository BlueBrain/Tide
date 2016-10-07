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

#include "LodSynchronizer.h"

#include "DataSource.h"
#include "QtImage.h"
#include "scene/ContentWindow.h"
#include "Tile.h"
#include "ZoomHelper.h"

#include <QTextStream>

LodSynchronizer::LodSynchronizer( const TileSwapPolicy policy )
    : TiledSynchronizer( policy )
{}

void LodSynchronizer::update( const ContentWindow& window,
                              const QRectF& visibleArea )
{
    const ZoomHelper helper( window );
    const uint lod = getLod( helper.getContentRect().size().toSize( ));
    const QSize tilesSurface = getDataSource().getTilesArea( lod );
    const QRectF visibleTilesArea = helper.toTilesArea( visibleArea,
                                                        tilesSurface );

    if( visibleTilesArea == _visibleTilesArea && lod == _lod )
        return;

    _visibleTilesArea = visibleTilesArea;

    if( lod != _lod )
    {
        _lod = lod;

        emit statisticsChanged();
        emit tilesAreaChanged();
    }

    setBackgroundTile( 0 );

    TiledSynchronizer::updateTiles( getDataSource(), false );
}

QSize LodSynchronizer::getTilesArea() const
{
    return getDataSource().getTilesArea( _lod );
}

QString LodSynchronizer::getStatistics() const
{
    QString stats;
    QTextStream stream( &stats );
    stream << "LOD:  " << _lod << "/" << getDataSource().getMaxLod();
    const QSize& area = getTilesArea();
    stream << "  res: " << area.width() << "x" << area.height();
    return stats;
}

ImagePtr LodSynchronizer::getTileImage( const uint tileIndex ) const
{
    return getDataSource().getTileImage( tileIndex );
}

TilePtr LodSynchronizer::getZoomContextTile() const
{
    const auto rect = getDataSource().getTileRect( 0 );
    return std::make_shared<Tile>( 0, rect );
}

uint LodSynchronizer::getLod( const QSize& targetDisplaySize ) const
{
    uint lod = 0;
    QSize nextLOD = getDataSource().getTilesArea( 1 );
    const uint maxLod = getDataSource().getMaxLod();
    while( targetDisplaySize.width() < nextLOD.width() &&
           targetDisplaySize.height() < nextLOD.height( ) &&
           lod < maxLod )
    {
        nextLOD = getDataSource().getTilesArea( ++lod + 1 );
    }
    return lod;
}

void LodSynchronizer::setBackgroundTile( const uint tileId )
{
    // Add an lod-0 tile always visible in the background to smooth LOD
    // transitions. TODO: implement a finer control to switch tiles after the
    // new LOD is ready [DISCL-345].

    if( !_ignoreSet.count( tileId ))
    {
        for( auto tile : _ignoreSet )
            emit removeTile( tile );
        _ignoreSet.clear();
    }

    if( _ignoreSet.empty( ))
    {
        _ignoreSet.insert( tileId );
        auto tile = std::make_shared<Tile>(
                        tileId, getDataSource().getTileRect( tileId ));
        tile->setSizePolicy( Tile::FillParent );
        emit addTile( tile );
    }
}
