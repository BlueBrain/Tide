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

#ifndef DATAPROVIDER_H
#define DATAPROVIDER_H

#include "synchronizers/ContentSynchronizer.h"
#include "types.h"

#include <QFutureWatcher>
#include <QList>
#include <QObject>

/**
 * Load tile images in parallel, synchronizing tiles swap and frame advance.
 */
class DataProvider : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DataProvider)

public:
    /** Construct a data provider. */
    DataProvider() = default;

    /** Destructor. */
    ~DataProvider();

    /**
     * Update the data sources when the scene has changed.
     *
     * @param scene with updated information.
     */
    void updateDataSources(const Scene& scene);

    /**
     * Create a ContentSynchronizer for the given window and view.
     *
     * A data source must alread exist for the target content; so this function
     * must be called *after* updateDataSources.
     */
    std::unique_ptr<ContentSynchronizer> createSynchronizer(
        const Window& window, deflect::View view);

    /**
     * Synchronize the swap of Tiles just before rendering.
     *
     * @param channel to synchonize swap accross all wall processes.
     */
    void synchronizeTilesSwap(WallToWallChannel& channel);

    /**
     * Synchronize the update of Tiles just before rendering.
     *
     * @param channel to synchonize the update accross all wall processes.
     */
    void synchronizeTilesUpdate(WallToWallChannel& channel);

public slots:
    /** Start loading a tile image asynchronously. */
    void loadAsync(TilePtr tile, deflect::View view);

    /** Update the frame for an existing PixelStream data source. */
    void setNewFrame(deflect::server::FramePtr frame);

signals:
    /** Emitted to request a new frame for a stream after a successful swap. */
    void requestPixelStreamFrame(QString uri);

    /** Emitted to request the pixel stream to close if an error occured. */
    void closePixelStream(QString uri);

    /** Emitted to request a new rendering after a tile image was loaded. */
    void imageLoaded();

private:
    using Watcher = QFutureWatcher<void>;
    QList<Watcher*> _watchers;

    std::map<QUuid, DataSourceSharedPtr> _dataSources;
    std::map<QString, QUuid> _streamSources;

    struct TileUpdateInfo
    {
        TileWeakPtr tile;
        deflect::View view;
    };
    using TileUpdateList = std::vector<TileUpdateInfo>;
    std::map<uint, TileUpdateList> _tileImageRequests;

    void _createOrUpdateDataSource(const Content& content, const QUuid& id);
    DataSourceSharedPtr _getOrCreateDataSource(const Content& content,
                                               const QUuid& id);

    void _updateTiles();
    void _startAsyncTileImageRequests(DataSourceSharedPtr source);
    void _handleStreamError(const QString& uri);
    void _load(DataSourceSharedPtr source, const TileUpdateList& tileList);
    void _handleFinished();
};

#endif
