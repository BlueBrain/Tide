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

#ifndef LODSYNCHRONIZER_H
#define LODSYNCHRONIZER_H

#include "TiledSynchronizer.h"

/**
 * Base synchronizer for tiled contents with multiple levels of detail.
 */
class LodSynchronizer : public TiledSynchronizer
{
    Q_OBJECT
    Q_DISABLE_COPY( LodSynchronizer )

public:
    /** Constructor. */
    LodSynchronizer( TileSwapPolicy policy );

    /** @copydoc ContentSynchronizer::updateTiles */
    void update( const ContentWindow& window,
                 const QRectF& visibleArea ) override;

    /** @copydoc ContentSynchronizer::getTilesArea */
    QSize getTilesArea() const override;

    /** @copydoc ContentSynchronizer::getStatistics */
    QString getStatistics() const override;

    /** @copydoc ContentSynchronizer::getTileImage */
    ImagePtr getTileImage( uint tileIndex ) const override;

    /** @copydoc ContentSynchronizer::getZoomContextTile */
    TilePtr getZoomContextTile() const override;

protected:
    /**
     * Get the appropriate LOD for a given display size.
     * @param targetDisplaySize The size at which the content will be displayed.
     */
    uint getLod( const QSize& targetDisplaySize ) const;

    /**
     * Derived classes must implement this method to return their data source.
     */
    virtual const DataSource& getDataSource() const = 0;

    /** Create the background tile to smooth the LOD change. */
    void setBackgroundTile( uint backgroundTileId );
};

#endif
