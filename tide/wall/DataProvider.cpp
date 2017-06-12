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
#include "scene/Content.h"
#include "scene/DisplayGroup.h"
#include "scene/MovieContent.h"
#include "scene/PDFContent.h"

#include "BasicSynchronizer.h"
#if TIDE_ENABLE_MOVIE_SUPPORT
#include "MovieSynchronizer.h"
#include "MovieUpdater.h"
#endif
#include "PixelStreamSynchronizer.h"
#include "PixelStreamUpdater.h"
#if TIDE_ENABLE_PDF_SUPPORT
#include "PDFSynchronizer.h"
#endif

#include <deflect/Frame.h>

#include <QtConcurrent>

namespace
{
template <typename Map>
std::shared_ptr<typename Map::mapped_type::element_type> _get(
    Map& map, const ContentWindow& window)
{
    const auto& id = window.getID();
    std::shared_ptr<typename Map::mapped_type::element_type> source;
    if (map.count(id))
        source = map[id].lock();

    if (!source)
    {
        const auto& uri = window.getContent()->getURI();
        source = std::make_shared<typename Map::mapped_type::element_type>(uri);
        map[id] = source;
    }
    return source;
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

    for (auto synchronizer : updater.synchronizers)
        synchronizer->updateTiles();
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

void DataProvider::updateDataSources(const DisplayGroup& group)
{
    std::set<QString> updatedStreams;
#if TIDE_ENABLE_MOVIE_SUPPORT
    std::set<QUuid> updatedMovies;
#endif

    for (const auto& window : group.getContentWindows())
    {
        const auto& content = *window->getContent();
        switch (content.getType())
        {
#if TIDE_ENABLE_MOVIE_SUPPORT
        case CONTENT_TYPE_MOVIE:
        {
            const auto& movie = static_cast<const MovieContent&>(content);
            _get(_movieSources, *window)->update(movie);
            updatedMovies.insert(window->getID());
        }
        break;
#endif
#if TIDE_ENABLE_PDF_SUPPORT
        case CONTENT_TYPE_PDF:
        {
            const auto& pdf = static_cast<const PDFContent&>(content);
            _get(_pdfSources, *window)->update(pdf);
        }
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

void DataProvider::synchronizeTilesSwap(WallToWallChannel& channel)
{
    for (auto stream : _streamSources)
        _synchronize(channel, *stream.second.lock());

#if TIDE_ENABLE_MOVIE_SUPPORT
    for (auto movie : _movieSources)
        _synchronize(channel, *movie.second.lock());
#endif
}

void DataProvider::loadAsync(ContentSynchronizerSharedPtr source,
                             TileWeakPtr tile)
{
    auto watcher = new Watcher;
    _watchers.append(watcher);
    connect(watcher, &Watcher::finished, this, &DataProvider::_handleFinished);
    watcher->setFuture(
        QtConcurrent::run([source, tile, this] { _load(source, tile); }));
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

std::shared_ptr<PixelStreamUpdater> DataProvider::_getStreamSource(
    const ContentWindow& window)
{
    const auto& uri = window.getContent()->getURI();
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

void DataProvider::_load(ContentSynchronizerSharedPtr source, TileWeakPtr tile_)
{
    TilePtr tile = tile_.lock();
    if (!tile)
    {
        put_flog(LOG_DEBUG, "Tile expired");
        return;
    }
    ImagePtr image;
    try
    {
        image = source->getDataSource().getTileImage(tile->getId(),
                                                     source->getView());
    }
    catch (...)
    {
        put_flog(LOG_ERROR, "An error occured with tile: %d", tile->getId());
        return;
    }
    if (!image)
    {
        put_flog(LOG_DEBUG, "Empty image for tile: %d", tile->getId());
        return;
    }

    QMetaObject::invokeMethod(tile.get(), "updateBackTexture",
                              Qt::QueuedConnection, Q_ARG(ImagePtr, image));

    emit imageLoaded();
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
    switch (window.getContent()->getType())
    {
#if TIDE_USE_TIFF
    case CONTENT_TYPE_IMAGE_PYRAMID:
        return make_unique<LodSynchronizer>(_get(_imagePyrSources, window));
#endif
#if TIDE_ENABLE_MOVIE_SUPPORT
    case CONTENT_TYPE_MOVIE:
    {
        auto updater = _get(_movieSources, window);
        updater->update(static_cast<const MovieContent&>(*window.getContent()));
        return make_unique<MovieSynchronizer>(updater, view);
    }
#endif
#if TIDE_ENABLE_PDF_SUPPORT
    case CONTENT_TYPE_PDF:
    {
        auto updater = _get(_pdfSources, window);
        updater->update(static_cast<const PDFContent&>(*window.getContent()));
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
