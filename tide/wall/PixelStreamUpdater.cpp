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

#include "log.h"
#include "network/WallToWallChannel.h"
#include "StreamImage.h"

#include <deflect/Frame.h>
#include <deflect/SegmentDecoder.h>

#include <QImage>
#include <QThreadStorage>

PixelStreamUpdater::PixelStreamUpdater( deflect::View view )
    : _view{ view }
    , _headerDecoder{ new deflect::SegmentDecoder }
{
    _swapSyncFrame.setCallback( std::bind( &PixelStreamUpdater::_onFrameSwapped,
                                           this, std::placeholders::_1 ));
}

PixelStreamUpdater::~PixelStreamUpdater()
{
}

void PixelStreamUpdater::synchronizeFramesSwap( WallToWallChannel& channel )
{
    if( !_readyToSwap )
        return;

    const auto versionCheckFunc = std::bind( &WallToWallChannel::checkVersion,
                                             &channel, std::placeholders::_1 );
    _swapSyncFrame.sync( versionCheckFunc );
}

QRect toRect( const deflect::SegmentParameters& params )
{
    return QRect( params.x, params.y, params.width, params.height );
}

QRect PixelStreamUpdater::getTileRect( const uint tileIndex ) const
{
    return toRect( _currentFrame->segments.at( tileIndex ).parameters );
}

TextureFormat PixelStreamUpdater::getTileFormat( const uint tileIndex ) const
{
#ifndef DEFLECT_USE_LEGACY_LIBJPEGTURBO
    const auto& segment = _currentFrame->segments.at( tileIndex );
    switch( segment.parameters.dataType )
    {
    case deflect::DataType::rgba:
        return TextureFormat::rgba;
    case deflect::DataType::yuv444:
        return TextureFormat::yuv444;
    case deflect::DataType::yuv422:
        return TextureFormat::yuv422;
    case deflect::DataType::yuv420:
        return TextureFormat::yuv420;
    case deflect::DataType::jpeg:
        switch( _headerDecoder->decodeType( segment ))
        {
        case deflect::ChromaSubsampling::YUV444:
            return TextureFormat::yuv444;
        case deflect::ChromaSubsampling::YUV422:
            return TextureFormat::yuv422;
        case deflect::ChromaSubsampling::YUV420:
            return TextureFormat::yuv420;
        }
    default:
        throw std::runtime_error( "Invalid data type for Tile" );
    }
#else
    Q_UNUSED( tileIndex );
    return TextureFormat::rgba;
#endif
}

QSize PixelStreamUpdater::getTilesArea( const uint lod ) const
{
    Q_UNUSED( lod );
    if( !_currentFrame )
        return QSize();
    return _currentFrame->computeDimensions();
}

ImagePtr PixelStreamUpdater::getTileImage( const uint tileIndex ) const
{
    if( !_currentFrame )
    {
        put_flog( LOG_ERROR, "No frames yet" );
        return ImagePtr();
    }

    const QReadLocker lock( &_mutex );

    auto& segment = _currentFrame->segments.at( tileIndex );
    if( segment.parameters.dataType == deflect::DataType::jpeg )
    {
        // turbojpeg handles need to be per thread, and this function may be
        // called from multiple threads
        static QThreadStorage< deflect::SegmentDecoder > decoder;
        try
        {
#ifndef DEFLECT_USE_LEGACY_LIBJPEGTURBO
            decoder.localData().decodeToYUV( segment );
#else
            decoder.localData().decode( segment );
#endif
        }
        catch( const std::runtime_error& e )
        {
            put_flog( LOG_ERROR, "Error decoding stream tile: '%s'", e.what( ));
            return ImagePtr();
        }
    }

    return std::make_shared<StreamImage>( _currentFrame, tileIndex );
}

Indices PixelStreamUpdater::computeVisibleSet( const QRectF& visibleTilesArea,
                                               const uint lod ) const
{
    Q_UNUSED( lod );

    Indices visibleSet;

    if( !_currentFrame || visibleTilesArea.isEmpty( ))
        return visibleSet;

    for( size_t i = 0; i < _currentFrame->segments.size(); ++i )
    {
        if( visibleTilesArea.intersects( toRect( _currentFrame->segments[i].parameters )))
            visibleSet.insert( i );
    }
    return visibleSet;
}

uint PixelStreamUpdater::getMaxLod() const
{
    return 0;
}

void PixelStreamUpdater::getNextFrame()
{
    _readyToSwap = true;
}

void PixelStreamUpdater::updatePixelStream( deflect::FramePtr frame )
{
    if( frame->view == _view || frame->view == deflect::View::mono ||
        ( _view == deflect::View::mono &&
          frame->view == deflect::View::left_eye ))
    {
        _swapSyncFrame.update( frame );
    }
}

void PixelStreamUpdater::_onFrameSwapped( deflect::FramePtr frame )
{
    _readyToSwap = false;

    std::sort( frame->segments.begin(), frame->segments.end(),
               []( const deflect::Segment& s1, const deflect::Segment& s2 )
        {
            return ( s1.parameters.y == s2.parameters.y ?
                         s1.parameters.x < s2.parameters.x :
                         s1.parameters.y < s2.parameters.y );
        } );

    {
        const QWriteLocker lock( &_mutex );
        _currentFrame = frame;
    }

    emit pictureUpdated();
    emit requestFrame( _currentFrame->uri );
}
