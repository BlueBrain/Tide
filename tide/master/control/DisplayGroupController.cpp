/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
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

#include "DisplayGroupController.h"

#include "ContentWindowController.h"
#include "DisplayGroup.h"
#include "LayoutEngine.h"

#include <QTransform>

DisplayGroupController::DisplayGroupController( DisplayGroup& group )
    : _group( group )
{}

void DisplayGroupController::remove( const QUuid windowId )
{
    auto window = _group.getContentWindow( windowId );
    if( !window )
        return;

    const auto focused = window->isFocused();
    _group.removeContentWindow( window );
    if( focused )
        updateFocusedWindowsCoordinates();
}

void DisplayGroupController::removeWindowLater( const QUuid windowId )
{
    auto window = _group.getContentWindow( windowId );
    if( !window )
        return;

    if( window->isFocused( ))
    {
        _group.removeFocusedWindow( window );
        updateFocusedWindowsCoordinates();
    }

    QMetaObject::invokeMethod( &_group, "removeContentWindow",
                               Qt::QueuedConnection,
                               Q_ARG( ContentWindowPtr, window ));
}

void DisplayGroupController::showFullscreen( const QUuid& id )
{
    ContentWindowPtr window = _group.getContentWindow( id );
    if( !window )
        return;

    exitFullscreen();

    const auto target = ContentWindowController::Coordinates::FULLSCREEN;
    ContentWindowController controller( *window, _group, target );
    controller.adjustSize( SizeState::SIZE_FULLSCREEN );

    _group.setFullscreenWindow( window );
}

void DisplayGroupController::exitFullscreen()
{
    _group.setFullscreenWindow( ContentWindowPtr( ));
}

void DisplayGroupController::focus( const QUuid& id )
{
    auto window = _group.getContentWindow( id );
    if( !window || window->isPanel() || _group.getFocusedWindows().count( window ))
        return;

    // Update focused windows coordinates BEFORE adding it for proper transition
    auto focusedWindows = _group.getFocusedWindows();
    focusedWindows.insert( window );
    LayoutEngine{ _group }.updateFocusedCoord( focusedWindows );

    _group.addFocusedWindow( window );
}

void DisplayGroupController::unfocus( const QUuid& id )
{
    auto window = _group.getContentWindow( id );
    if( !window || !_group.getFocusedWindows().count( window ))
        return;

    _group.removeFocusedWindow( window );
    updateFocusedWindowsCoordinates();

    // Make sure the window dimensions are re-adjusted to the new zoom level
    ContentWindowController{ *window, _group }.scale( window->center(), 0.0 );
}

void DisplayGroupController::unfocusAll()
{
    while( !_group.getFocusedWindows().empty( ))
        unfocus( (*_group.getFocusedWindows().begin( ))->getID( ));
}

void DisplayGroupController::moveWindowToFront( const QUuid id )
{
    _group.moveContentWindowToFront( _group.getContentWindow( id ));
}

void DisplayGroupController::scale( const QSizeF& factor )
{
    const QTransform t = QTransform::fromScale( factor.width(),
                                                factor.height( ));

    for( ContentWindowPtr window : _group.getContentWindows( ))
        window->setCoordinates( t.mapRect( window->getCoordinates( )));

    _group.setCoordinates( t.mapRect( _group.getCoordinates( )));
}

void DisplayGroupController::adjust( const QSizeF& maxGroupSize )
{
    auto targetSize = _group.size().scaled( maxGroupSize, Qt::KeepAspectRatio );
    const qreal scaleFactor = targetSize.width() / _group.width();
    scale( QSizeF( scaleFactor, scaleFactor ));
}

void DisplayGroupController::reshape( const QSizeF& newSize )
{
    adjust( newSize );
    _extend( newSize );
}

void DisplayGroupController::denormalize( const QSizeF& targetSize )
{
    if( _group.getCoordinates() != UNIT_RECTF )
        throw std::runtime_error( "Target DisplayGroup is not normalized!" );

    const qreal aspectRatio = _estimateAspectRatio();
    const QSizeF scaleFactor = QSizeF( aspectRatio, 1.0 ).scaled( targetSize,
                                                          Qt::KeepAspectRatio );
    scale( scaleFactor );
    // Make sure aspect ratio is 100% correct for all windows - some may be
    // slightly off due to numerical imprecisions in the xml state file
    adjustWindowsAspectRatioToContent();
}

void DisplayGroupController::adjustWindowsAspectRatioToContent()
{
    for( ContentWindowPtr window : _group.getContentWindows( ))
    {
        QSizeF exactSize = window->getContent()->getDimensions();
        exactSize.scale( window->getCoordinates().size(), Qt::KeepAspectRatio );
        window->setWidth( exactSize.width( ));
        window->setHeight( exactSize.height( ));
    }
}

QRectF DisplayGroupController::estimateSurface() const
{
    QRectF area( UNIT_RECTF );
    for( ContentWindowPtr contentWindow : _group.getContentWindows( ))
        area = area.united( contentWindow->getCoordinates( ));
    area.setTopLeft( QPointF( 0.0, 0.0 ));

    return area;
}

void DisplayGroupController::updateFocusedWindowsCoordinates()
{
    LayoutEngine{ _group }.updateFocusedCoord( _group.getFocusedWindows( ));
}

void DisplayGroupController::_extend( const QSizeF& newSize )
{
    const QSizeF offset = 0.5 * ( newSize - _group.getCoordinates().size( ));

    _group.setWidth( newSize.width( ));
    _group.setHeight( newSize.height( ));

    const QTransform t = QTransform::fromTranslate( offset.width(),
                                                    offset.height( ));
    for( ContentWindowPtr window : _group.getContentWindows( ))
        window->setCoordinates( t.mapRect( window->getCoordinates( )));
}

qreal DisplayGroupController::_estimateAspectRatio() const
{
    qreal averageAR = 0.0;
    for( ContentWindowPtr window : _group.getContentWindows( ))
    {
        const qreal windowAR = window->width() / window->height();
        averageAR += window->getContent()->getAspectRatio() / windowAR;
    }
    averageAR /= _group.getContentWindows().size();
    return averageAR;
}
