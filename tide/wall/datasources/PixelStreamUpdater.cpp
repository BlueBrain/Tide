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

#include "PixelStreamUpdater.h"

#include "data/StreamImage.h"
#include "network/WallToWallChannel.h"
#include "tools/PixelStreamAssembler.h"
#include "tools/PixelStreamPassthrough.h"
#include "utils/log.h"

#include <deflect/server/Frame.h>
#include <deflect/server/TileDecoder.h>

#include <QImage>
#include <QThreadStorage>

namespace
{
void _splitByView(const deflect::server::Tiles& tiles,
                  deflect::server::Tiles& leftOrMono,
                  deflect::server::Tiles& right)
{
    std::partition_copy(tiles.begin(), tiles.end(), std::back_inserter(right),
                        std::back_inserter(leftOrMono), [](const auto& tile) {
                            return tile.view == deflect::View::right_eye;
                        });
    assert(right.empty() || right.size() == leftOrMono.size());
}

void _sortByChannelAndPosition(deflect::server::Tiles& tiles)
{
    std::sort(tiles.begin(), tiles.end(), [](const auto& t1, const auto& t2) {
        return t1.channel == t2.channel
                   ? (t1.y == t2.y ? t1.x < t2.x : t1.y < t2.y)
                   : t1.channel < t2.channel;
    });
}
}

PixelStreamUpdater::PixelStreamUpdater(const QString& uri)
    : _uri{uri}
    , _headerDecoder{new deflect::server::TileDecoder}
{
    _swapSyncFrame.setCallback(std::bind(&PixelStreamUpdater::_onFrameSwapped,
                                         this, std::placeholders::_1));
}

PixelStreamUpdater::~PixelStreamUpdater()
{
}

const QString& PixelStreamUpdater::getUri() const
{
    return _uri;
}

ImagePtr PixelStreamUpdater::getTileImage(const uint tileIndex,
                                          const deflect::View view) const
{
    if (!_frameLeftOrMono)
    {
        print_log(LOG_ERROR, LOG_STREAM, "No frames yet");
        return ImagePtr();
    }

    // guard against frame swap during asynchronous readings
    const QReadLocker frameLock(&_frameMutex);

    const bool rightEye = view == deflect::View::right_eye;
    const bool rightFrame = rightEye && !_frameRight->tiles.empty();
    const auto& processor = rightFrame ? _processRight : _processorLeft;

    // turbojpeg handles need to be per thread, and this function may be
    // called from multiple threads
    static QThreadStorage<deflect::server::TileDecoder> tileDecoders;
    try
    {
        return processor->getTileImage(tileIndex, tileDecoders.localData());
    }
    catch (const std::runtime_error& e)
    {
        print_log(LOG_ERROR, LOG_STREAM, "Error decoding stream tile: '%s'",
                  e.what());
        return ImagePtr();
    }
}

QRect PixelStreamUpdater::getTileRect(const uint tileIndex) const
{
    return _processorLeft->getTileRect(tileIndex);
}

QSize PixelStreamUpdater::getTilesArea(const uint lod, const uint channel) const
{
    Q_UNUSED(lod);
    if (!_frameLeftOrMono)
        return QSize();
    return _frameLeftOrMono->computeDimensions(channel);
}

Indices PixelStreamUpdater::computeVisibleSet(const QRectF& visibleTilesArea,
                                              const uint lod,
                                              const uint channel) const
{
    Q_UNUSED(lod);

    if (!_frameLeftOrMono || visibleTilesArea.isEmpty())
        return Indices{};

    return _processorLeft->computeVisibleSet(visibleTilesArea, channel);
}

uint PixelStreamUpdater::getMaxLod() const
{
    return 0;
}

void PixelStreamUpdater::allowNextFrame()
{
    _readyToSwap = true;
}

void PixelStreamUpdater::synchronizeFrameAdvance(WallToWallChannel& channel)
{
    if (!_readyToSwap)
        return;

    const auto versionCheckFunc = std::bind(&WallToWallChannel::checkVersion,
                                            &channel, std::placeholders::_1);
    _swapSyncFrame.sync(versionCheckFunc);
}

void PixelStreamUpdater::setNextFrame(deflect::server::FramePtr frame)
{
    assert(frame->uri == getUri());

    _swapSyncFrame.update(frame);
}

void PixelStreamUpdater::_onFrameSwapped(deflect::server::FramePtr frame)
{
    _readyToSwap = false;

    auto leftOrMono = std::make_shared<deflect::server::Frame>();
    auto right = std::make_shared<deflect::server::Frame>();

    _splitByView(frame->tiles, leftOrMono->tiles, right->tiles);
    _sortByChannelAndPosition(leftOrMono->tiles);
    _sortByChannelAndPosition(right->tiles);

    {
        const QWriteLocker frameLock(&_frameMutex);
        _frameLeftOrMono = std::move(leftOrMono);
        _frameRight = std::move(right);
        _createFrameProcessors();
    }

    emit pictureUpdated();
    emit requestFrame(frame->uri);
}

void PixelStreamUpdater::_createFrameProcessors()
{
    try
    {
        if (!_frameLeftOrMono->tiles.empty())
            _processorLeft.reset(new PixelStreamAssembler(_frameLeftOrMono));
        else
            _processorLeft.reset();

        if (!_frameRight->tiles.empty())
            _processRight.reset(new PixelStreamAssembler(_frameRight));
        else
            _processRight.reset();
    }
    catch (const std::runtime_error&)
    {
        _processorLeft.reset(new PixelStreamPassthrough(_frameLeftOrMono));
        _processRight.reset(new PixelStreamPassthrough(_frameRight));
    }
}
