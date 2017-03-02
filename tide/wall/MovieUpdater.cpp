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

#include "MovieUpdater.h"

#include "data/FFMPEGFrame.h"
#include "data/FFMPEGMovie.h"
#include "data/FFMPEGPicture.h"
#include "log.h"
#include "network/WallToWallChannel.h"
#include "scene/MovieContent.h"


MovieUpdater::MovieUpdater( const QString& uri )
    : _ffmpegMovie( new FFMPEGMovie( uri ))
{
    // Observed bug [DISCL-295]: opening a movie might fail on WallProcesses
    // despite correctly reading metadata on the MasterProcess.
    // bool FFMPEGMovie::openVideoStreamDecoder(): could not open codec
    // error: -11 Resource temporarily unavailable
    if( !_ffmpegMovie->isValid( ))
        put_flog( LOG_WARN, "Movie is invalid: %s",
                  uri.toLocal8Bit().constData( ));
}

MovieUpdater::~MovieUpdater() {}

void MovieUpdater::update( const MovieContent& movie, const bool visible )
{
    _paused = movie.getControlState() & STATE_PAUSED;
    _loop = movie.getControlState() & STATE_LOOP;
    _visible = visible;
    _skipping = movie.isSkipping();
    _skipPosition = movie.getPosition();
}

QRect MovieUpdater::getTileRect( const uint tileIndex ) const
{
    Q_UNUSED( tileIndex );
    return QRect( 0, 0, _ffmpegMovie->getWidth(), _ffmpegMovie->getHeight( ));
}

TextureFormat MovieUpdater::getTileFormat( const uint tileIndex ) const
{
    Q_UNUSED( tileIndex );
    return _ffmpegMovie->getFormat();
}

QSize MovieUpdater::getTilesArea( const uint lod ) const
{
    Q_UNUSED( lod );
    return getTileRect( 0 ).size();
}

ImagePtr MovieUpdater::getTileImage( const uint tileIndex ) const
{
    Q_UNUSED( tileIndex );
    double timestamp;
    {
        const QMutexLocker lock( &_mutex );
        timestamp = _sharedTimestamp;
    }

    ImagePtr image = _ffmpegMovie->getFrame( timestamp );

    const bool loopBack = _loop && !image;
    if( loopBack )
        image = _ffmpegMovie->getFrame( 0.0 );

    {
        const QMutexLocker lock( &_mutex );
        _currentPosition = _ffmpegMovie->getPosition();
        // stay inSync for start != 0.0 and loop conditions
        _sharedTimestamp = _currentPosition;
        // WAR a risk of deadlock when skipping movies with incorrect duration
        _loopedBack = loopBack;
    }
    return image;
}

Indices MovieUpdater::computeVisibleSet( const QRectF& visibleTilesArea,
                                         const uint lod ) const
{
    Q_UNUSED( lod );

    if( visibleTilesArea.isEmpty( ))
        return Indices();

    return { 0 };
}

uint MovieUpdater::getMaxLod() const
{
    return 0;
}

void MovieUpdater::lastFrameDone()
{
    _fpsCounter.tick();

    _requestNewFrame = false;
    _lastFrameDone = true;
}

bool MovieUpdater::canRequestNewFrame() const
{
    return _requestNewFrame;
}

QString MovieUpdater::getStatistics() const
{
    const QString fps =
            QString::number( 1.0 / _ffmpegMovie->getFrameDuration(), 'g', 3 );
    const QString progress = QString::number( getPosition() * 100.0, 'g', 3 );
    return QString( "%1 / %2 fps %3 %").arg(_fpsCounter.toString())
                                       .arg( fps )
                                       .arg( progress );
}

qreal MovieUpdater::getPosition() const
{
    return _sharedTimestamp / _ffmpegMovie->getDuration();
}

bool MovieUpdater::isSkipping() const
{
    return _skipping;
}

qreal MovieUpdater::getSkipPosition() const
{
    return _skipPosition / _ffmpegMovie->getDuration();
}

bool MovieUpdater::advanceToNextFrame( WallToWallChannel& channel )
{
    const double frameDuration = _ffmpegMovie->getFrameDuration();

    // protect _sharedTimestamp & _currentPosition from getTileImage()
    const QMutexLocker lock( &_mutex );

    // Jump to the skip position
    if( _skipping && !_loopedBack )
        _sharedTimestamp = _skipPosition;

    // If any visible updater is out-of-sync, only update those ones. This
    // causes a seek in the movie to _sharedTimestamp. The time stands still in
    // this case to avoid seeking of all processes if this seek takes longer
    // than frameDuration.
    const bool inSync =
            std::abs( _sharedTimestamp - _currentPosition ) <= frameDuration;
    _lastFrameDone = channel.allReady( _lastFrameDone );
    if( !channel.allReady( !_visible || inSync ))
    {
        _timer.resetTime( channel.getTime( ));
        if( !_lastFrameDone )
            return false;
        _lastFrameDone = false;
        _requestNewFrame = _visible && !inSync;
        return true;
    }

    // Don't advance time if paused or skipping
    if( _paused || _skipping )
    {
        _timer.resetTime( channel.getTime( ));
        return false;
    }

    // If everybody is in sync, do a proper increment and throttle to movie
    // frame duration and decode speed accordingly.
    _timer.setCurrentTime( channel.getTime( ));
    _elapsedTime += _timer.getElapsedTimeInSeconds();
    if( !_lastFrameDone || _elapsedTime < frameDuration )
        return false;

    // advance to the next frame, keep correct elapsedTime as vsync frequency
    // of this function might not match movie frequency.
    _sharedTimestamp = _currentPosition + frameDuration;
    _elapsedTime -= frameDuration;

    // Always exchange timestamp for processes where _currentPosition is not
    // advancing to allow seek if visible again.
    _exchangeSharedTimestamp( channel, inSync );

    _lastFrameDone = false;
    _requestNewFrame = _visible;
    return true;
}

void MovieUpdater::_exchangeSharedTimestamp( WallToWallChannel& channel,
                                             const bool inSync )
{
    const int leader = channel.electLeader( _visible && inSync );
    if( leader < 0 )
        return;

    if( leader == channel.getRank( ))
        channel.broadcast( _sharedTimestamp );
    else
        _sharedTimestamp = channel.receiveTimestampBroadcast( leader );
}
