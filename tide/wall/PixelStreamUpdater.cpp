/*********************************************************************/
/* Copyright (c) 2015-2017, EPFL/Blue Brain Project                  */
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

#include "PixelStreamAssembler.h"
#include "PixelStreamPassthrough.h"
#include "StreamImage.h"
#include "log.h"
#include "network/WallToWallChannel.h"

#include <deflect/Frame.h>
#include <deflect/SegmentDecoder.h>

#include <QImage>
#include <QThreadStorage>

namespace
{
void _splitByView(const deflect::Segments& segments,
                  deflect::Segments& leftOrMono, deflect::Segments& right)
{
    std::partition_copy(segments.begin(), segments.end(),
                        std::back_inserter(right),
                        std::back_inserter(leftOrMono),
                        [](const deflect::Segment& segment) {
                            return segment.view == deflect::View::right_eye;
                        });
    assert(right.empty() || right.size() == leftOrMono.size());
}

void _sortByPosition(deflect::Segments& segments)
{
    std::sort(segments.begin(), segments.end(),
              [](const deflect::Segment& s1, const deflect::Segment& s2) {
                  return (s1.parameters.y == s2.parameters.y
                              ? s1.parameters.x < s2.parameters.x
                              : s1.parameters.y < s2.parameters.y);
              });
}
}

PixelStreamUpdater::PixelStreamUpdater()
    : _headerDecoder{new deflect::SegmentDecoder}
{
    _swapSyncFrame.setCallback(std::bind(&PixelStreamUpdater::_onFrameSwapped,
                                         this, std::placeholders::_1));
}

PixelStreamUpdater::~PixelStreamUpdater()
{
}

void PixelStreamUpdater::synchronizeFrameAdvance(WallToWallChannel& channel)
{
    if (!_readyToSwap)
        return;

    const auto versionCheckFunc = std::bind(&WallToWallChannel::checkVersion,
                                            &channel, std::placeholders::_1);
    _swapSyncFrame.sync(versionCheckFunc);
}

QRect PixelStreamUpdater::getTileRect(const uint tileIndex) const
{
    return _processorLeft->getTileRect(tileIndex);
}

QSize PixelStreamUpdater::getTilesArea(const uint lod) const
{
    Q_UNUSED(lod);
    if (!_frameLeftOrMono)
        return QSize();
    return _frameLeftOrMono->computeDimensions();
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
    const bool rightFrame = rightEye && !_frameRight->segments.empty();
    const auto& processor = rightFrame ? _processRight : _processorLeft;

    // turbojpeg handles need to be per thread, and this function may be
    // called from multiple threads
    static QThreadStorage<deflect::SegmentDecoder> segmentDecoders;
    try
    {
        return processor->getTileImage(tileIndex, segmentDecoders.localData());
    }
    catch (const std::runtime_error& e)
    {
        print_log(LOG_ERROR, LOG_STREAM, "Error decoding stream tile: '%s'",
                  e.what());
        return ImagePtr();
    }
}

Indices PixelStreamUpdater::computeVisibleSet(const QRectF& visibleTilesArea,
                                              const uint lod) const
{
    Q_UNUSED(lod);

    if (!_frameLeftOrMono || visibleTilesArea.isEmpty())
        return Indices{};

    return _processorLeft->computeVisibleSet(visibleTilesArea);
}

uint PixelStreamUpdater::getMaxLod() const
{
    return 0;
}

void PixelStreamUpdater::getNextFrame()
{
    _readyToSwap = true;
}

void PixelStreamUpdater::updatePixelStream(deflect::FramePtr frame)
{
    _swapSyncFrame.update(frame);
}

void PixelStreamUpdater::_onFrameSwapped(deflect::FramePtr frame)
{
    _readyToSwap = false;

    auto leftOrMono = std::make_shared<deflect::Frame>();
    auto right = std::make_shared<deflect::Frame>();

    _splitByView(frame->segments, leftOrMono->segments, right->segments);
    _sortByPosition(leftOrMono->segments);
    _sortByPosition(right->segments);

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
        if (!_frameLeftOrMono->segments.empty())
            _processorLeft.reset(new PixelStreamAssembler(_frameLeftOrMono));
        else
            _processorLeft.reset();

        if (!_frameRight->segments.empty())
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
