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

#include "ContentSynchronizer.h"
#include "PixelStreamUpdater.h"
#include "Tile.h"

#include "log.h"

#include <QtConcurrent>
#include <deflect/Frame.h>

DataProvider::DataProvider(const deflect::View view)
    : _view{view}
{
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

PixelStreamUpdaterSharedPtr DataProvider::getStreamDataSource(
    const QString& uri)
{
    PixelStreamUpdaterSharedPtr updater;
    if (_streamUpdaters.count(uri))
        updater = _streamUpdaters[uri].lock();

    if (!updater)
    {
        updater = std::make_shared<PixelStreamUpdater>(_view);
        connect(updater.get(), &PixelStreamUpdater::requestFrame, this,
                &DataProvider::requestFrame);
        _streamUpdaters[uri] = PixelStreamUpdaterWeakPtr(updater);

        // Fix DISCL-382: New frames are requested after showing the current
        // one,
        // but it's conditional to _streamUpdaters[uri] in setNewFrame(), hence
        // request a frame once we have a PixelStreamUpdater.
        emit requestFrame(uri);
    }

    return updater;
}

void DataProvider::loadAsync(ContentSynchronizerSharedPtr source,
                             TileWeakPtr tile)
{
    Watcher* watcher = new Watcher;
    _watchers.append(watcher);
    connect(watcher, &Watcher::finished, this, &DataProvider::_handleFinished);
    watcher->setFuture(
        QtConcurrent::run([source, tile, this] { _load(source, tile); }));
}

void DataProvider::setNewFrame(deflect::FramePtr frame)
{
    if (!_streamUpdaters.count(frame->uri))
        return;

    if (auto updater = _streamUpdaters[frame->uri].lock())
        updater->updatePixelStream(frame);
    else
        _streamUpdaters.erase(frame->uri);
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
        image = source->getTileImage(tile->getId());
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
    emit imageLoaded(image, tile_);
}

void DataProvider::_handleFinished()
{
    Watcher* watcher = static_cast<Watcher*>(sender());
    _watchers.removeOne(watcher);
    watcher->deleteLater();
}
