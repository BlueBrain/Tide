/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
/*                     Raphael.Dumusc@epfl.ch                        */
/*                     Daniel.Nachbaur@epfl.ch                       */
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

#include "Options.h"

#include "Content.h"

// false-positive on qt signals for Q_PROPERTY notifiers
// cppcheck-suppress uninitMemberVar
Options::Options()
    : _alphaBlendingEnabled( false )
    , _autoFocusPixelStreams( true )
    , _showClock( false )
    , _showContentTiles( false )
    , _showControlArea( true )
    , _showStreamingStatistics( false )
    , _showTestPattern( false )
    , _showTouchPoints( true )
    , _showWindowBorders( true )
    , _showZoomContext( true )
{}

bool Options::isAlphaBlendingEnabled() const
{
    return _alphaBlendingEnabled;
}

bool Options::getAutoFocusPixelStreams() const
{
    return _autoFocusPixelStreams;
}

bool Options::getShowClock() const
{
    return _showClock;
}

bool Options::getShowContentTiles() const
{
    return _showContentTiles;
}

bool Options::getShowControlArea() const
{
    return _showControlArea;
}

bool Options::getShowStatistics() const
{
    return _showStreamingStatistics;
}

bool Options::getShowTestPattern() const
{
    return _showTestPattern;
}

bool Options::getShowTouchPoints() const
{
    return _showTouchPoints;
}

bool Options::getShowWindowBorders() const
{
    return _showWindowBorders;
}

bool Options::getShowZoomContext() const
{
    return _showZoomContext;
}

QColor Options::getBackgroundColor() const
{
    return _backgroundColor;
}

ContentPtr Options::getBackgroundContent() const
{
    return _backgroundContent;
}

void Options::enableAlphaBlending( const bool set )
{
    if( _alphaBlendingEnabled == set )
        return;

    _alphaBlendingEnabled = set;
    emit alphaBlendingEnabledChanged( set );
    emit updated( shared_from_this( ));
}

void Options::setAutoFocusPixelStreams( const bool set )
{
    if( _autoFocusPixelStreams == set )
        return;

    _autoFocusPixelStreams = set;
    emit autoFocusPixelStreamsChanged( set );
    emit updated( shared_from_this( ));
}

void Options::setShowClock( const bool set )
{
    if( _showClock == set )
        return;

    _showClock = set;
    emit showClockChanged( set );
    emit updated( shared_from_this( ));
}

void Options::setShowContentTiles( const bool set )
{
    if( _showContentTiles == set )
        return;

    _showContentTiles = set;
    emit showContentTilesChanged( set );
    emit updated( shared_from_this( ));
}

void Options::setShowControlArea( const bool set )
{
    if( _showControlArea == set )
        return;

    _showControlArea = set;
    emit showControlAreaChanged( );
    emit updated( shared_from_this( ));
}

void Options::setShowStatistics( const bool set )
{
    if( _showStreamingStatistics == set )
        return;

    _showStreamingStatistics = set;
    emit showStatisticsChanged( set );
    emit updated( shared_from_this( ));
}

void Options::setShowTestPattern( const bool set )
{
    if( _showTestPattern == set )
        return;

    _showTestPattern = set;
    emit showTestPatternChanged( set );
    emit updated( shared_from_this( ));
}

void Options::setShowTouchPoints( const bool set )
{
    if( _showTouchPoints == set )
        return;

    _showTouchPoints = set;
    emit showTouchPointsChanged( set );
    emit updated( shared_from_this( ));
}

void Options::setShowWindowBorders( const bool set )
{
    if( _showWindowBorders == set )
        return;

    _showWindowBorders = set;
    emit showWindowBordersChanged( set );
    emit updated( shared_from_this( ));
}

void Options::setShowZoomContext( const bool set )
{
    if( _showZoomContext == set )
        return;

    _showZoomContext = set;
    emit showZoomContextChanged( set );
    emit updated( shared_from_this( ));
}

void Options::setBackgroundColor( const QColor color )
{
    if( color == _backgroundColor )
        return;

    _backgroundColor = color;
    emit updated( shared_from_this( ));
}

void Options::setBackgroundContent( ContentPtr content )
{
    if( _backgroundContent == content )
        return;

    _backgroundContent = content;
    emit updated( shared_from_this( ));
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
void Options::moveToThread( QThread* thread )
{
    if( _backgroundContent )
        _backgroundContent->moveToThread( thread );
    QObject::moveToThread( thread );
}
#pragma GCC diagnostic pop
#pragma clang diagnostic pop
