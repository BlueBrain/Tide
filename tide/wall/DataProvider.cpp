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

#include "DataProvider.h"

#include "config.h"
#include "datasources/DataSourceFactory.h"
#include "datasources/PixelStreamUpdater.h"
#include "network/WallToWallChannel.h"
#include "qml/Tile.h"
#include "scene/Background.h"
#include "scene/Scene.h"
#include "scene/Window.h"
#include "synchronizers/ContentSynchronizerFactory.h"
#include "utils/log.h"

#include <deflect/server/Frame.h>

#include <QtConcurrent>

namespace
{
template <typename Map>
void remove_unused(Map& map, const std::set<typename Map::key_type>& validKeys)
{
    auto it = map.begin();
    while (it != map.end())
    {
        if (validKeys.count(it->first))
            ++it;
        else
            it = map.erase(it);
    }
}

inline auto cast_to_stream_source(DataSourceSharedPtr source)
{
    return std::dynamic_pointer_cast<PixelStreamUpdater>(source);
}
}

DataProvider::~DataProvider()
{
    for (auto watcher : _watchers)
    {
        watcher->disconnect(this);
        watcher->waitForFinished();
        delete watcher;
    }
}

void DataProvider::updateDataSources(const Scene& scene)
{
    // Synchronized contents (such as streams and movies) must be added and
    // removed synchronously here. Otherwise, in synchronizeTilesSwap() locking
    // the weak pointer may succeed on processes that are asynchronously getting
    // a tile image but fail on the others, causing a deadlock.

    std::set<QUuid> updatedSources;

    for (const auto& surface : scene.getSurfaces())
    {
        const auto& background = surface.getBackground();
        if (auto content = background.getContent())
            _createOrUpdateDataSource(*content, background.getContentUUID());
        updatedSources.insert(background.getContentUUID());
    }

    for (const auto& window : scene.getWindows())
    {
        const auto& content = window->getContent();
        _createOrUpdateDataSource(content, window->getID());
        updatedSources.insert(window->getID());
    }

    remove_unused(_dataSources, updatedSources);
}

std::unique_ptr<ContentSynchronizer> DataProvider::createSynchronizer(
    const Window& window, const deflect::View view)
{
    auto source = _dataSources.at(window.getID());
    auto synchronizer =
        ContentSynchronizerFactory::create(window.getContent(), view, source);

    connect(synchronizer.get(), &ContentSynchronizer::requestTileUpdate, this,
            &DataProvider::loadAsync);

    return synchronizer;
}

void DataProvider::synchronizeTilesSwap(WallToWallChannel& channel)
{
    for (auto dataSource : _dataSources)
    {
        auto& source = *dataSource.second;
        if (source.isDynamic()) // movies and pixelstreams
        {
            if (channel.allReady(source.synchronizers.canSwapTiles()))
            {
                source.synchronizers.swapTiles();
                source.allowNextFrame();
            }
        }
    }
}

void DataProvider::synchronizeTilesUpdate(WallToWallChannel& channel)
{
    for (auto dataSource : _dataSources)
        dataSource.second->synchronizeFrameAdvance(channel);
    _updateTiles();
}

void DataProvider::loadAsync(TilePtr tile, deflect::View view)
{
    // Group the requests for a single tile from multiple WallWindows for the
    // data source currently being processed.
    // This ensures that getTileImage is never called more than once per Tile.
    _tileImageRequests[tile->getId()].push_back({tile, view});
}

void DataProvider::setNewFrame(deflect::server::FramePtr frame)
{
    const auto id = _streamSources[frame->uri];
    if (!_dataSources.count(id))
        return;

    if (auto stream = cast_to_stream_source(_dataSources[id]))
        stream->setNextFrame(frame);
}

void DataProvider::_createOrUpdateDataSource(const Content& content,
                                             const QUuid& id)
{
    _getOrCreateDataSource(content, id)->update(content);
}

DataSourceSharedPtr DataProvider::_getOrCreateDataSource(const Content& content,
                                                         const QUuid& id)
{
    if (!_dataSources.count(id))
    {
        _dataSources[id] = DataSourceFactory::create(content);
        if (auto stream = cast_to_stream_source(_dataSources[id]))
        {
            _streamSources[content.getUri()] = id;
            connect(stream.get(), &PixelStreamUpdater::requestFrame, this,
                    &DataProvider::requestPixelStreamFrame);

            // request the first frame now that the data source is ready to
            // accept it.
            emit requestPixelStreamFrame(content.getUri());
        }
    }
    return _dataSources[id];
}

void DataProvider::_updateTiles()
{
    auto it = _dataSources.begin();
    while (it != _dataSources.end())
    {
        // The following results in loadAsync() being called one or multiple
        // times, filling _tileImageRequests with the tiles from the
        // different WallWindows for this data source.
        try
        {
            _tileImageRequests.clear();

            auto source = it->second;
            source->synchronizers.updateTiles(); // may throw

            // Start the asynchronous loading of images for this data source
            // and clear the list of requests for the next data source.
            _startTileImageRequests(source);
            ++it;
        }
        catch (const std::exception& e)
        {
            print_log(LOG_ERROR, LOG_GENERAL,
                      "closing data source due to exception: %s", e.what());
            it = _dataSources.erase(it);

            if (auto stream = cast_to_stream_source(it->second))
                _handleStreamError(stream->getUri());
        }
    }
}

void DataProvider::_startTileImageRequests(DataSourceSharedPtr source)
{
    for (const auto& tileRequest : _tileImageRequests)
    {
        auto watcher = new Watcher;
        _watchers.append(watcher);
        connect(watcher, &Watcher::finished, this,
                &DataProvider::_handleFinished);
        watcher->setFuture(QtConcurrent::run(
            [ this, source, tilesToUpdate = tileRequest.second ] {
                _load(source, tilesToUpdate);
            }));
    }
}

void DataProvider::_handleStreamError(const QString& uri)
{
    print_log(LOG_ERROR, LOG_STREAM, "closing pixel stream %s",
              uri.toLocal8Bit().constData());
    _streamSources.erase(uri);
    emit closePixelStream(uri);
}

void DataProvider::_load(DataSourceSharedPtr source,
                         const TileUpdateList& tiles)
{
    // Request image only once for each view
    std::map<deflect::View, ImagePtr> image;

    for (const auto& it : tiles)
    {
        if (auto tile = it.tile.lock())
        {
            const auto view = it.view;
            const auto id = tile->getId();
            if (!image[view])
            {
                try
                {
                    image[view] = source->getTileImage(id, view);
                }
                catch (...)
                {
                    print_log(LOG_ERROR, LOG_GENERAL,
                              "An error occured with tile: %d", id);
                    return;
                }
                if (!image[view])
                {
                    print_log(LOG_DEBUG, LOG_GENERAL,
                              "Empty image for tile: %d", id);
                    return;
                }
                emit imageLoaded(); // Keep RenderController active
            }
            QMetaObject::invokeMethod(tile.get(), "updateBackTexture",
                                      Qt::QueuedConnection,
                                      Q_ARG(ImagePtr, image[view]),
                                      Q_ARG(TilePtr, tile));
        }
        else
            print_log(LOG_DEBUG, LOG_GENERAL, "Tile expired");
    }
}

void DataProvider::_handleFinished()
{
    auto watcher = static_cast<Watcher*>(sender());
    _watchers.removeOne(watcher);
    watcher->deleteLater();
}
