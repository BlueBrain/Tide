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

#include "Tile.h"
#include "config.h"
#include "network/WallToWallChannel.h"
#include "scene/Background.h"
#include "scene/MultiChannelContent.h"
#include "scene/Scene.h"
#include "scene/Window.h"
#include "utils/log.h"

#include "BasicSynchronizer.h"
#include "LodSynchronizer.h"
#include "PixelStreamSynchronizer.h"
#include "PixelStreamUpdater.h"

#if TIDE_ENABLE_MOVIE_SUPPORT
#include "MovieSynchronizer.h"
#include "MovieUpdater.h"
#include "scene/MovieContent.h"
#endif
#if TIDE_ENABLE_PDF_SUPPORT
#include "PDFSynchronizer.h"
#include "scene/PDFContent.h"
#endif

#include <deflect/server/Frame.h>

#include <QtConcurrent>

namespace
{
template <typename Map>
std::shared_ptr<typename Map::mapped_type::element_type> _getOrCreate(
    Map& map, const QUuid& id, const QString& uri)
{
    std::shared_ptr<typename Map::mapped_type::element_type> source;
    if (map.count(id))
        source = map[id].lock();

    if (!source)
    {
        source = std::make_shared<typename Map::mapped_type::element_type>(uri);
        map[id] = source;
    }
    return source;
}

template <typename Map>
std::shared_ptr<typename Map::mapped_type::element_type> _getOrCreate(
    Map& map, const Window& window)
{
    const auto& id = window.getID();
    const auto& uri = window.getContent().getUri();
    return _getOrCreate(map, id, uri);
}

template <typename Map>
void _removeUnused(Map& map, const std::set<typename Map::key_type>& validKeys)
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

template <typename Updater>
void _synchronize(WallToWallChannel& channel, Updater& updater)
{
    bool swap = true;
    for (auto synchronizer : updater.synchronizers)
        swap = swap && synchronizer->canSwapTiles();

    if (channel.allReady(swap))
    {
        for (auto synchronizer : updater.synchronizers)
            synchronizer->swapTiles();
        updater.getNextFrame();
    }

    updater.synchronizeFrameAdvance(channel);
}

bool _isMovie(const Background& background)
{
    if (auto content = background.getContent())
        return content->getType() == ContentType::movie;
    return false;
}

template <typename T>
inline std::shared_ptr<T> getSharedPtr(const std::weak_ptr<T>& ptr)
{
    return ptr.lock();
}
template <typename T>
inline std::shared_ptr<T> getSharedPtr(const std::shared_ptr<T>& ptr)
{
    return ptr;
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

std::unique_ptr<ContentSynchronizer> DataProvider::createSynchronizer(
    const Window& window, const deflect::View view)
{
    auto synchronizer = _makeSynchronizer(window, view);

    connect(synchronizer.get(), &ContentSynchronizer::requestTileUpdate, this,
            &DataProvider::loadAsync);

    return synchronizer;
}

void DataProvider::updateDataSources(const Scene& scene)
{
// Streams and movies are synchronized contents, so they must be added and
// removed synchronously here. Otherwise, in synchronizeTilesSwap() locking
// the weak pointer may succeed on processes that are asynchronously getting
// a tile image but fail on the others, causing a deadlock.

#if TIDE_ENABLE_MOVIE_SUPPORT
    std::set<QUuid> updatedMovies;
#endif
    std::set<QString> updatedStreams;

    for (const auto& surface : scene.getSurfaces())
    {
        const auto& background = surface.getBackground();
        if (auto content = background.getContent())
            _updateDataSource(*content, background.getContentUUID());
#if TIDE_ENABLE_MOVIE_SUPPORT
        if (_isMovie(background))
            updatedMovies.insert(background.getContentUUID());
#endif
    }

    for (const auto& window : scene.getWindows())
    {
        const auto& content = window->getContent();
        _updateDataSource(content, window->getID());

        switch (content.getType())
        {
#if TIDE_ENABLE_MOVIE_SUPPORT
        case ContentType::movie:
            updatedMovies.insert(window->getID());
            break;
#endif
        case ContentType::pixel_stream:
        case ContentType::webbrowser:
            updatedStreams.insert(content.getUri());
            break;
        default:
            break; /** nothing to do */
        }
    }

#if TIDE_ENABLE_MOVIE_SUPPORT
    _removeUnused(_movieSources, updatedMovies);
#endif
    _removeUnused(_streamSources, updatedStreams);
}

void DataProvider::synchronizeTilesSwap(WallToWallChannel& channel)
{
    for (auto stream : _streamSources)
        _synchronize(channel, *stream.second);
    _updateTiles(_streamSources);

#if TIDE_ENABLE_MOVIE_SUPPORT
    for (auto movie : _movieSources)
        _synchronize(channel, *movie.second);
    _updateTiles(_movieSources);
#endif

    _updateTiles(_imageSources);

#if TIDE_USE_TIFF
    _updateTiles(_imagePyrSources);
#endif

#if TIDE_ENABLE_PDF_SUPPORT
    _updateTiles(_pdfSources);
#endif

    _updateTiles(_svgSources);
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
    if (!_streamSources.count(frame->uri))
        return;

    _streamSources[frame->uri]->updatePixelStream(frame);
}

void DataProvider::_updateDataSource(const Content& content, const QUuid& id)
{
    switch (content.getType())
    {
#if TIDE_ENABLE_MOVIE_SUPPORT
    case ContentType::movie:
    {
        const auto& movie = static_cast<const MovieContent&>(content);
        _getOrCreateMovieSource(movie.getUri(), id)->update(movie);
    }
    break;
#endif
#if TIDE_ENABLE_PDF_SUPPORT
    case ContentType::pdf:
    {
        const auto& pdf = static_cast<const PDFContent&>(content);
        if (_pdfSources.count(id))
            _pdfSources.at(id).lock()->update(pdf);
    }
    break;
#endif
    case ContentType::pixel_stream:
    case ContentType::webbrowser:
    {
        _getOrCreateStreamSource(content.getUri());
    }
    break;
    default:
        break; /** nothing to do */
    }
}

template <typename DataSources>
void DataProvider::_updateTiles(DataSources& dataSources)
{
    auto it = dataSources.begin();
    while (it != dataSources.end())
    {
        if (auto source = getSharedPtr(it->second))
        {
            // The following results in loadAsync() being called one or multiple
            // times, filling _tileImageRequests with the tiles from the
            // different WallWindows for this data source.
            try
            {
                for (auto synchronizer : source->synchronizers)
                    synchronizer->updateTiles();
            }
            catch (const std::exception& exc)
            {
                _handleError<decltype(source)>(it->first, exc);
                it = dataSources.erase(it);
                continue;
            }

            // Start the asynchronous loading of images for this data source
            // and clear the list of requests for the next data source.
            _processTileImageRequests(source);
            ++it;
        }
        else
        {
            print_log(LOG_DEBUG, LOG_GENERAL, "Removing invalid source");
            it = dataSources.erase(it);
        }
    }
}

template <>
void DataProvider::_handleError<std::shared_ptr<PixelStreamUpdater>, QString>(
    QString uri, const std::exception& exc)
{
    print_log(LOG_ERROR, LOG_STREAM, "%s, closing pixel stream %s", exc.what(),
              uri.toLocal8Bit().constData());
    emit closePixelStream(uri);
}

void DataProvider::_processTileImageRequests(DataSourcePtr source)
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
    _tileImageRequests.clear();
}

#if TIDE_ENABLE_MOVIE_SUPPORT
std::shared_ptr<MovieUpdater> DataProvider::_getOrCreateMovieSource(
    const QString& uri, const QUuid& id)
{
    auto it = _movieSources.find(id);
    if (it == _movieSources.end())
    {
        it = _movieSources.emplace(id, std::make_shared<MovieUpdater>(uri))
                 .first;
    }
    return it->second;
}
#endif

std::shared_ptr<PixelStreamUpdater> DataProvider::_getOrCreateStreamSource(
    const QString& uri)
{
    auto it = _streamSources.find(uri);
    if (it == _streamSources.end())
    {
        it = _streamSources.emplace(uri, std::make_shared<PixelStreamUpdater>())
                 .first;
        connect(it->second.get(), &PixelStreamUpdater::requestFrame, this,
                &DataProvider::requestFrame);
        // Fix DISCL-382: New frames are requested after showing the current
        // one, but it's conditional to _streamSources[id] in setNewFrame(),
        // hence request a frame once we have a PixelStreamUpdater.
        emit requestFrame(uri);
    }
    return it->second;
}

void DataProvider::_load(DataSourcePtr source, const TileUpdateList& tiles)
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

std::unique_ptr<ContentSynchronizer> DataProvider::_makeSynchronizer(
    const Window& window, const deflect::View view)
{
    switch (window.getContent().getType())
    {
#if TIDE_USE_TIFF
    case ContentType::image_pyramid:
    {
        auto source = _getOrCreate(_imagePyrSources, window);
        return std::make_unique<LodSynchronizer>(source);
    }
#endif
#if TIDE_ENABLE_MOVIE_SUPPORT
    case ContentType::movie:
    {
        auto source = _movieSources.at(window.getID());
        source->update(static_cast<const MovieContent&>(window.getContent()));
        return std::make_unique<MovieSynchronizer>(source, view);
    }
#endif
#if TIDE_ENABLE_PDF_SUPPORT
    case ContentType::pdf:
    {
        auto source = _getOrCreate(_pdfSources, window);
        source->update(static_cast<const PDFContent&>(window.getContent()));
        return std::make_unique<PDFSynchronizer>(source);
    }
#endif
    case ContentType::pixel_stream:
    case ContentType::webbrowser:
    {
        const auto& content = window.getContent();
        auto source = _streamSources.at(content.getUri());
        const auto channel =
            static_cast<const MultiChannelContent&>(content).getChannel();
        return std::make_unique<PixelStreamSynchronizer>(source, view, channel);
    }
    case ContentType::svg:
    {
        auto source = _getOrCreate(_svgSources, window);
        return std::make_unique<LodSynchronizer>(source);
    }
    case ContentType::texture:
    {
        auto source = _getOrCreate(_imageSources, window);
        return std::make_unique<BasicSynchronizer>(source);
    }
    default:
        throw std::runtime_error("No ContentSynchronizer for ContentType");
    }
}
