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

#include "config.h"
#include "types.h"

#include "ContentSynchronizer.h"
#if TIDE_USE_TIFF
#include "ImagePyramidDataSource.h"
#endif
#include "ImageSource.h"
#if TIDE_ENABLE_PDF_SUPPORT
#include "PDFTiler.h"
#endif
#include "SVGTiler.h"

#include <QFutureWatcher>
#include <QList>
#include <QObject>

/**
 * Load image data in parallel.
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

    /** @return a ContentSynchronizer for the given content. */
    std::unique_ptr<ContentSynchronizer> createSynchronizer(
        const ContentWindow& window, deflect::View view);

    /**
     * Update the data sources with information from a new scene.
     *
     * @param scene with updated information for the movies/pdfs/streams.
     */
    void updateDataSources(const Scene& scene);

    /**
     * Synchronize the swap of Tiles just before rendering for movies/streams.
     *
     * @param channel to synchonize swap accross all wall processes.
     */
    void synchronizeTilesSwap(WallToWallChannel& channel);

public slots:
    /** Load an image asynchronously. */
    void loadAsync(TilePtr tile, deflect::View view);

    /** Add a new frame. */
    void setNewFrame(deflect::server::FramePtr frame);

signals:
    /** Emitted to request a new frame after a successful swap. */
    void requestFrame(QString uri);

    /** Emitted to request the pixel stream to close. */
    void closePixelStream(QString uri);

    /** Emitted to request a new rendering after a tile image was loaded. */
    void imageLoaded();

private:
    using Watcher = QFutureWatcher<void>;
    QList<Watcher*> _watchers;

#if TIDE_ENABLE_MOVIE_SUPPORT
    std::map<QUuid, std::shared_ptr<MovieUpdater>> _movieSources;
#endif
    std::map<QString, std::shared_ptr<PixelStreamUpdater>> _streamSources;

    std::map<QUuid, std::weak_ptr<ImageSource>> _imageSources;
#if TIDE_USE_TIFF
    std::map<QUuid, std::weak_ptr<ImagePyramidDataSource>> _imagePyrSources;
#endif
#if TIDE_ENABLE_PDF_SUPPORT
    std::map<QUuid, std::weak_ptr<PDFTiler>> _pdfSources;
#endif
    std::map<QUuid, std::weak_ptr<SVGTiler>> _svgSources;

    void _updateDataSource(const Content& content, const QUuid& id);

    struct TileUpdateInfo
    {
        TileWeakPtr tile;
        deflect::View view;
    };
    using TileUpdateList = std::vector<TileUpdateInfo>;
    std::map<uint, TileUpdateList> _tileImageRequests;

    using DataSourcePtr = std::shared_ptr<DataSource>;
    void _processTileImageRequests(DataSourcePtr source);

#if TIDE_ENABLE_MOVIE_SUPPORT
    std::shared_ptr<MovieUpdater> _getOrCreateMovieSource(const QString& uri,
                                                          const QUuid& id);
#endif
    std::shared_ptr<PixelStreamUpdater> _getOrCreateStreamSource(
        const QString& uri);
    void _load(DataSourcePtr source, const TileUpdateList& tileList);
    void _handleFinished();
    std::unique_ptr<ContentSynchronizer> _makeSynchronizer(
        const ContentWindow& window, deflect::View view);

    template <typename DataSources>
    void _updateTiles(DataSources& dataSources);

    template <typename DataSource, typename URIType>
    void _handleError(URIType, const std::exception&)
    {
    }
};

#endif
