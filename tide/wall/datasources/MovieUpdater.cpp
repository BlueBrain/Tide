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

#include "MovieUpdater.h"

#include "data/FFMPEGFrame.h"
#include "data/FFMPEGMovie.h"
#include "data/FFMPEGPicture.h"
#include "network/WallToWallChannel.h"
#include "scene/MovieContent.h"
#include "utils/log.h"

#include <cmath>

namespace
{
auto _splitSideBySide(const FFMPEGPicture& image)
{
    const auto width = image.getWidth() / 2;
    const auto height = image.getHeight();
    const auto format = image.getFormat();

    auto left = std::make_shared<FFMPEGPicture>(width, height, format);
    auto right = std::make_shared<FFMPEGPicture>(width, height, format);

    const uint numTextures = (format == TextureFormat::rgba) ? 1 : 3 /*YUV*/;
    for (uint texture = 0; texture < numTextures; ++texture)
    {
        const auto uvSize = image.getTextureSize(texture);
        const size_t lineWidth = uvSize.width();
        const size_t targetWidth = lineWidth / 2;
        for (size_t y = 0; y < size_t(uvSize.height()); ++y)
        {
            const auto input = image.getData(texture) + y * lineWidth;
            const auto outLeft = left->getData(texture) + y * targetWidth;
            const auto outRight = right->getData(texture) + y * targetWidth;
            std::copy(input, input + targetWidth, outLeft);
            std::copy(input + targetWidth, input + lineWidth, outRight);
        }
    }
    return std::make_pair(left, right);
}

auto _splitSideBySide(std::shared_ptr<FFMPEGPicture> image)
{
    if (!image)
        return std::make_pair(image, image);
    return _splitSideBySide(*image);
}
}

MovieUpdater::MovieUpdater(const QString& uri)
    : _uri{uri}
{
    try
    {
        _ffmpegMovie = std::make_unique<FFMPEGMovie>(uri);
        _duration = _ffmpegMovie->getDuration();
        _frameDuration = _ffmpegMovie->getFrameDuration();
    }
    catch (const std::runtime_error& e)
    {
        // Observed bug [DISCL-295]: opening a movie might fail on WallProcesses
        // despite correctly reading metadata on the MasterProcess.
        // bool FFMPEGMovie::openVideoStreamDecoder(): could not open codec
        // error: -11 Resource temporarily unavailable
        print_log(LOG_WARN, LOG_AV, "Movie file can't be opened: %s - %s",
                  uri.toLocal8Bit().constData(), e.what());
    }
}

MovieUpdater::~MovieUpdater() = default;

QString MovieUpdater::getUri() const
{
    return _uri;
}

void MovieUpdater::update(const Content& content)
{
    const auto& movie = dynamic_cast<const MovieContent&>(content);

    _paused = movie.isPaused();
    _loop = movie.isLooping();
    _skipping = movie.isSkipping();
    _skipPosition = movie.getPosition();
    // must be received from master in case _ffmpegMovie is invalid on this node
    _duration = movie.getDuration();
    _frameDuration = movie.getFrameDuration();
}

QRect MovieUpdater::getTileRect(const uint tileIndex) const
{
    Q_UNUSED(tileIndex);

    if (!_ffmpegMovie)
        return QRect();

    return QRect(0, 0, _ffmpegMovie->getWidth(), _ffmpegMovie->getHeight());
}

QSize MovieUpdater::getTilesArea(const uint lod, const uint channel) const
{
    Q_UNUSED(lod);
    Q_UNUSED(channel);

    return getTileRect(0).size();
}

Indices MovieUpdater::computeVisibleSet(const QRectF& visibleTilesArea,
                                        const uint lod,
                                        const uint channel) const
{
    Q_UNUSED(lod);
    Q_UNUSED(channel);

    if (!_ffmpegMovie || visibleTilesArea.isEmpty())
        return Indices();

    return {0};
}

ImagePtr MovieUpdater::getTileImage(const uint tileIndex,
                                    const deflect::View view) const
{
    Q_UNUSED(tileIndex);

    if (!_ffmpegMovie)
        throw std::runtime_error("Movie is invalid");

    // WAR bug: concurrent calls to this function may occur when the movie was
    // obstruced by another window and becomes visible again, resulting in a
    // segfault in: FFMPEGMovie::getFrame() ->
    // FFMPEGVideoStream::decodePictureForLastPacket() -> sws_scale().
    const QMutexLocker lockGetImage(&_getImageMutex);

    if (_ffmpegMovie->isStereo() && _pictureLeftOrMono && _pictureRight)
    {
        return view == deflect::View::right_eye ? _pictureRight
                                                : _pictureLeftOrMono;
    }
    else if (_pictureLeftOrMono)
        return _pictureLeftOrMono;

    double timestamp;
    {
        const QMutexLocker lock(&_mutex);
        timestamp = _sharedTimestamp;
    }

    auto image = _ffmpegMovie->getFrame(timestamp);

    const bool loopBack = _loop && !image;
    if (loopBack)
        image = _ffmpegMovie->getFrame(0.0);

    // Warning: in rare cases image may still be null at this point

    {
        const QMutexLocker lock(&_mutex);
        _currentPosition = _ffmpegMovie->getPosition();
        // stay inSync for start != 0.0 and loop conditions
        _sharedTimestamp = _currentPosition;
        // WAR a risk of deadlock when skipping movies with incorrect duration
        _loopedBack = loopBack;
    }
    if (_ffmpegMovie->isStereo())
    {
        std::tie(_pictureLeftOrMono, _pictureRight) = _splitSideBySide(image);
        return view == deflect::View::right_eye ? _pictureRight
                                                : _pictureLeftOrMono;
    }
    _pictureLeftOrMono = image;
    return image;
}

uint MovieUpdater::getMaxLod() const
{
    return 0;
}

QString MovieUpdater::getStatistics() const
{
    const auto movieFps = QString::number(1.0 / _frameDuration, 'g', 3);
    const auto progress = QString::number(getPosition() * 100.0, 'g', 3);
    return QString("%2 fps %3 %").arg(movieFps, progress);
}

qreal MovieUpdater::getPosition() const
{
    return _sharedTimestamp / _duration;
}

bool MovieUpdater::isSkipping() const
{
    return _skipping;
}

bool MovieUpdater::isPaused() const
{
    return _paused;
}

qreal MovieUpdater::getSkipPosition() const
{
    return _skipPosition / _duration;
}

void MovieUpdater::allowNextFrame()
{
    _readyForNextFrame = true;
}

void MovieUpdater::synchronizeFrameAdvance(WallToWallChannel& channel)
{
    const bool visible = synchronizers.haveVisibleTiles();
    const double frameDuration = _frameDuration;

    bool inSync = false;
    {
        // protect _sharedTimestamp & _currentPosition from getTileImage()
        const QMutexLocker lock(&_mutex);

        // Jump to the skip position
        if (_skipping && !_loopedBack)
            _sharedTimestamp = _skipPosition;

        inSync = std::abs(_sharedTimestamp - _currentPosition) <= frameDuration;
    }

    // If any visible updater is out-of-sync, only update those ones. This
    // causes a seek in the movie to _sharedTimestamp. The time stands still in
    // this case to avoid seeking of all processes if this seek takes longer
    // than frameDuration.
    if (!channel.allReady(inSync || !visible))
    {
        _timer.resetTime(channel.getTime());
        if (_readyForNextFrame)
            _triggerFrameUpdate();
        return;
    }

    // Don't advance time if paused or skipping
    if (_paused || _skipping)
    {
        _timer.resetTime(channel.getTime());
        if (_skipping && _readyForNextFrame)
            if (channel.allReady(!inSync || !visible))
                _triggerFrameUpdate();
        return;
    }

    // If everybody is in sync, do a proper increment and throttle to movie
    // frame duration and decode speed accordingly.
    _timer.setCurrentTime(channel.getTime());
    _elapsedTime += _timer.getElapsedTimeInSeconds();
    if (_elapsedTime < frameDuration || !_readyForNextFrame)
        return;
    {
        // protect _sharedTimestamp & _currentPosition from getTileImage()
        const QMutexLocker lock(&_mutex);

        // advance to the next frame, keep correct elapsedTime as vsync
        // frequency of this function might not match movie frequency.
        _sharedTimestamp = _currentPosition + frameDuration;
        _elapsedTime -= frameDuration;

        // Always exchange timestamp for processes where _currentPosition is not
        // advancing to allow seek if visible again.
        _exchangeSharedTimestamp(channel, visible && inSync);
    }
    // unlock _mutex before to avoid deadlocks
    _triggerFrameUpdate();
}

void MovieUpdater::_triggerFrameUpdate()
{
    _readyForNextFrame = false;
    _pictureLeftOrMono.reset();
    _pictureRight.reset();
    emit pictureUpdated();
}

void MovieUpdater::_exchangeSharedTimestamp(WallToWallChannel& channel,
                                            const bool isCandidate)
{
    const int leader = channel.electLeader(isCandidate);
    if (leader < 0)
        return;

    if (leader == channel.getRank())
        channel.broadcast(_sharedTimestamp);
    else
        _sharedTimestamp = channel.receiveTimestampBroadcast(leader);
}
