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

#include "DataProvider.h"

#include "Tile.h"
#include "config.h"
#include "log.h"
#include "network/WallToWallChannel.h"
#include "scene/Background.h"
#include "scene/ContentWindow.h"

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

#include <deflect/Frame.h>

#include <QtConcurrent>

namespace
{
template <typename Map>
std::shared_ptr<typename Map::mapped_type::element_type> _get(
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
std::shared_ptr<typename Map::mapped_type::element_type> _get(
    Map& map, const ContentWindow& window)
{
    const auto& id = window.getID();
    const auto& uri = window.getContent().getURI();
    return _get(map, id, uri);
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
        return content->getType() == CONTENT_TYPE_MOVIE;
    return false;
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
    const ContentWindow& window, const deflect::View view)
{
    auto synchronizer = _makeSynchronizer(window, view);

    connect(synchronizer.get(), &ContentSynchronizer::requestTileUpdate, this,
            &DataProvider::loadAsync);

    return synchronizer;
}

void DataProvider::updateDataSources(const ContentWindowPtrs& windows)
{
#if TIDE_ENABLE_MOVIE_SUPPORT
    std::set<QUuid> updatedMovies;
    if (_background && _isMovie(*_background))
        updatedMovies.insert(_background->getContentUUID());
#endif
    std::set<QString> updatedStreams;

    for (const auto& window : windows)
    {
        const auto& content = window->getContent();
        _updateDataSource(content, window->getID());

        switch (content.getType())
        {
#if TIDE_ENABLE_MOVIE_SUPPORT
        case CONTENT_TYPE_MOVIE:
            updatedMovies.insert(window->getID());
            break;
#endif
        case CONTENT_TYPE_PIXEL_STREAM:
        case CONTENT_TYPE_WEBBROWSER:
            updatedStreams.insert(content.getURI());
            break;
        default:
            break; /** nothing to do */
        }
    }

    // Streams and movies must be removed synchronously here. Otherwise, in
    // synchronizeTilesSwap() locking the weak pointer may succeed on processes
    // that are asynchronously getting a tile image but fail on the others,
    // causing a deadlock.
    _removeUnused(_streamSources, updatedStreams);
#if TIDE_ENABLE_MOVIE_SUPPORT
    _removeUnused(_movieSources, updatedMovies);
#endif
}

void DataProvider::updateDataSource(BackgroundPtr background)
{
    if (auto content = background->getContent())
        _updateDataSource(*content, background->getContentUUID());

#if TIDE_ENABLE_MOVIE_SUPPORT
    // Movies must be removed synchronously here, see detailed comment in
    // updateDataSources(windows).
    if (_background && _isMovie(*_background) &&
        _background->getContentUUID() != background->getContentUUID())
    {
        _movieSources.erase(_background->getContentUUID());
    }
#endif
    _background = std::move(background);
}

void DataProvider::synchronizeTilesSwap(WallToWallChannel& channel)
{
    for (auto stream : _streamSources)
        _synchronize(channel, *stream.second.lock());
    _updateTiles(_streamSources);

#if TIDE_ENABLE_MOVIE_SUPPORT
    for (auto movie : _movieSources)
        _synchronize(channel, *movie.second.lock());
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
    _tileImageRequests[tile->getId()].emplace_back(tile, view);
}

void DataProvider::setNewFrame(deflect::FramePtr frame)
{
    if (!_streamSources.count(frame->uri))
        return;

    if (auto updater = _streamSources[frame->uri].lock())
        updater->updatePixelStream(frame);
    else
        _streamSources.erase(frame->uri);
}

void DataProvider::_updateDataSource(const Content& content, const QUuid& id)
{
    switch (content.getType())
    {
#if TIDE_ENABLE_MOVIE_SUPPORT
    case CONTENT_TYPE_MOVIE:
    {
        const auto& movie = static_cast<const MovieContent&>(content);
        _get(_movieSources, id, content.getURI())->update(movie);
    }
    break;
#endif
#if TIDE_ENABLE_PDF_SUPPORT
    case CONTENT_TYPE_PDF:
    {
        const auto& pdf = static_cast<const PDFContent&>(content);
        _get(_pdfSources, id, content.getURI())->update(pdf);
    }
    break;
#endif
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
        if (auto source = it->second.lock())
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
        const auto& tilesToUpdate = tileRequest.second;
        watcher->setFuture(QtConcurrent::run(
            [this, source, tilesToUpdate] { _load(source, tilesToUpdate); }));
    }
    _tileImageRequests.clear();
}

std::shared_ptr<PixelStreamUpdater> DataProvider::_getStreamSource(
    const ContentWindow& window)
{
    const auto& uri = window.getContent().getURI();
    std::shared_ptr<PixelStreamUpdater> updater;
    if (_streamSources.count(uri))
        updater = _streamSources[uri].lock();

    if (!updater)
    {
        updater = std::make_shared<PixelStreamUpdater>();
        connect(updater.get(), &PixelStreamUpdater::requestFrame, this,
                &DataProvider::requestFrame);
        _streamSources[uri] = updater;

        // Fix DISCL-382: New frames are requested after showing the current
        // one, but it's conditional to _streamSources[id] in setNewFrame(),
        // hence request a frame once we have a PixelStreamUpdater.
        emit requestFrame(uri);
    }

    return updater;
}

void DataProvider::_load(DataSourcePtr source, const TileUpdateList& tiles)
{
    // Request image only once for each view
    std::map<deflect::View, ImagePtr> image;

    for (const auto& it : tiles)
    {
        if (auto tile = it.first.lock())
        {
            const auto view = it.second;
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
    const ContentWindow& window, const deflect::View view)
{
    switch (window.getContent().getType())
    {
#if TIDE_USE_TIFF
    case CONTENT_TYPE_IMAGE_PYRAMID:
        return make_unique<LodSynchronizer>(_get(_imagePyrSources, window));
#endif
#if TIDE_ENABLE_MOVIE_SUPPORT
    case CONTENT_TYPE_MOVIE:
    {
        auto updater = _get(_movieSources, window);
        updater->update(static_cast<const MovieContent&>(window.getContent()));
        return make_unique<MovieSynchronizer>(updater, view);
    }
#endif
#if TIDE_ENABLE_PDF_SUPPORT
    case CONTENT_TYPE_PDF:
    {
        auto updater = _get(_pdfSources, window);
        updater->update(static_cast<const PDFContent&>(window.getContent()));
        return make_unique<PDFSynchronizer>(updater);
    }
#endif
    case CONTENT_TYPE_PIXEL_STREAM:
    case CONTENT_TYPE_WEBBROWSER:
        return make_unique<PixelStreamSynchronizer>(_getStreamSource(window),
                                                    view);
    case CONTENT_TYPE_SVG:
        return make_unique<LodSynchronizer>(_get(_svgSources, window));
    case CONTENT_TYPE_TEXTURE:
        return make_unique<BasicSynchronizer>(_get(_imageSources, window));
    default:
        throw std::runtime_error("No ContentSynchronizer for ContentType");
    }
}
