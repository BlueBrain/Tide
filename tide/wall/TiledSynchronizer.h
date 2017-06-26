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

#ifndef TILEDSYNCHRONIZER_H
#define TILEDSYNCHRONIZER_H

#include "ContentSynchronizer.h"

#include <QObject>

/**
 * A base synchronizer used for tiled content types with optional LOD.
 */
class TiledSynchronizer : public ContentSynchronizer
{
    Q_OBJECT
    Q_DISABLE_COPY(TiledSynchronizer)

public:
    enum TileSwapPolicy
    {
        SwapTilesIndependently,
        SwapTilesSynchronously
    };

    /** Constructor */
    explicit TiledSynchronizer(TileSwapPolicy policy);

    /** @copydoc ContentSynchronizer::onSwapReady */
    void onSwapReady(TilePtr tile) override;

    /**
     * @return true if TileSwapPolicy is SwapTilesSynchronously and tiles are
     *         ready to be swapped.
     */
    bool canSwapTiles() const override;

    /**
     * Call to swap tiles when TileSwapPolicy is SwapTilesSynchronously.
     *
     * Should only be called when canSwapTiles returns true on all processes.
     */
    void swapTiles() override;

protected:
    uint _lod;
    QRectF _visibleTilesArea;
    Indices _ignoreSet;

    /**
     * Update the tiles, adding or removing them from the view.
     *
     * @param updateExistingTiles also update the texture and coordinates of the
     *        tiles which are already visible. If TileSwapPolicy is
     *        SwapTilesSynchronously, the updated textures will only be shown
     *        after a successful call to swapTiles().
     */
    void updateTiles(bool updateExistingTiles);

private:
    TileSwapPolicy _policy;

    Indices _visibleSet;

    bool _syncSwapPending;
    std::set<TilePtr> _tilesReadyToSwap;
    Indices _tilesReadySet;
    Indices _syncSet;
    Indices _removeLaterSet;

    void _removeTile(size_t tileIndex);
};

#endif
