/*********************************************************************/
/* Copyright (c) 2015-2018, EPFL/Blue Brain Project                  */
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

#ifndef CONTENTSYNCHRONIZER_H
#define CONTENTSYNCHRONIZER_H

#include "types.h"

#include <QObject>

/**
 * Interface for synchronizing QML content rendering.
 *
 * ContentSynchronizer informs the QML engine when new frames are ready and
 * swaps them synchronously.
 */
class ContentSynchronizer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ContentSynchronizer)
    Q_PROPERTY(QString statistics READ getStatistics NOTIFY statisticsChanged)
    Q_PROPERTY(bool zoomContextVisible READ getZoomContextVisible WRITE
                   setZoomContextVisible NOTIFY zoomContextVisibleChanged)
    Q_PROPERTY(uint lod READ getLod NOTIFY lodChanged)
    Q_PROPERTY(uint lodCount READ getLodCount CONSTANT)

public:
    /** Constructor */
    ContentSynchronizer() = default;

    /** Virtual destructor */
    virtual ~ContentSynchronizer() = default;

    /** Update the Content. */
    virtual void update(const Window& window, const QRectF& visibleArea) = 0;

    /**
     * Update the tiles.
     * Call addTile, updateTile and zoomContextTileChanged only in this method.
     */
    virtual void updateTiles() = 0;

    /**
     * Swap the image before rendering (useful only for synchronized contents).
     *
     * Called only when canSwapTiles returns true on all processes.
     */
    virtual void swapTiles() = 0;

    /** @return true if tiles are ready to be swapped. */
    virtual bool canSwapTiles() const = 0;

    /**
     * @return the total area covered by the tiles for a given LOD.
     * @note has to be a method + notifier instead of a Q_PROPERTY because there
     *       is no support for QList<QSize> in Qml.
     */
    Q_INVOKABLE QSize getTilesArea(const uint lod) const
    {
        if (lod >= getLodCount())
            throw std::logic_error("invalid LOD requested");
        return _getTilesArea(lod);
    }

    /** Get statistics about this Content. */
    virtual QString getStatistics() const = 0;

    /** Check if the zoom context tile is visible in Qml. */
    bool getZoomContextVisible() const { return _zoomContextVisible; }
    /** Get the data source. */
    virtual const DataSource& getDataSource() const = 0;

    /** @return a new tile for the zoom context, or nullptr if unsupported. */
    virtual TilePtr createZoomContextTile() const { return TilePtr(); }
    /** Get the view for this synchronizer. */
    virtual deflect::View getView() const { return deflect::View::mono; }
    /** @return true if at least one tile is visible for this synchronizer. */
    virtual bool hasVisibleTiles() const = 0;

    /** @return the current level of detail. */
    virtual uint getLod() const { return 0; }
    /** @return the number of level of detail. */
    virtual uint getLodCount() const { return 1; }
public slots:
    /**
     * Called when a tile is ready to swap.
     * Specfific implementation can choose to swap the tile immediately or
     * delay the swap until a later synchronization point.
     */
    virtual void onSwapReady(TilePtr tile) = 0;

    /** Called when a tile has to be updated, re-emits requestTileUpdate. */
    void onRequestNextFrame(TilePtr tile)
    {
        emit requestTileUpdate(tile, getView());
    }

    /** Set by the Qml ZoomContext element. */
    void setZoomContextVisible(const bool zoomContextVisible)
    {
        if (_zoomContextVisible == zoomContextVisible)
            return;

        _zoomContextVisible = zoomContextVisible;
        emit zoomContextVisibleChanged();
    }

signals:
    /** Notifier for the tiles areas property. @see getTilesArea() */
    void tilesAreasChanged();

    /** Notifier for the statistics property. */
    void statisticsChanged();

    /** Notifier for the zoomContextVisible property. */
    void zoomContextVisibleChanged();

    /** Notify the window to add a tile. */
    void addTile(TilePtr tile, uint zOrder);

    /** Notify the window to remove a tile. */
    void removeTile(uint tileId);

    /** Notify to update a tile's coordinates. */
    void updateTile(uint tileId, QRect coordinates);

    /** Request an update of a specific tile. */
    void requestTileUpdate(TilePtr tile, deflect::View view);

    /** Notify that the zoom context tile has changed and must be recreated. */
    void zoomContextTileChanged(bool visible);

    /** Notify that the level of detail has changed. */
    void lodChanged(uint lod);

private:
    bool _zoomContextVisible = false;

    /** @return the total area covered by the tiles for a given LOD. */
    virtual QSize _getTilesArea(uint lod) const = 0;
};

#endif
