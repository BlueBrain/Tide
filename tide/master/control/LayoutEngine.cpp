/*********************************************************************/
/* Copyright (c) 2015, EPFL/Blue Brain Project                       */
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

#include "LayoutEngine.h"

#include "ContentWindow.h"
#include "control/ContentWindowController.h"
#include "DisplayGroup.h"

#include <QTransform>

namespace
{
const qreal INSIDE_MARGIN_RELATIVE = 0.02;
const qreal SIDEBAR_WITH_REL_TO_DISPLAYGROUP_HEIGHT = 0.3 * 0.3;
const qreal WINDOW_CONTROLS_MARGIN_PX = 200.0;
const qreal WINDOW_SPACING_PX = 80.0;
}

LayoutEngine::LayoutEngine( const DisplayGroup& group )
    : _displayGroup( group )
{
}

LayoutEngine::~LayoutEngine() {}

typedef std::list<QRectF> WindowCoordinates;

qreal _computeAggregatedWidth( const WindowCoordinates& coords )
{
    qreal width = 0.0;
    for( auto coord : coords )
        width += coord.width();
    return width;
}

WindowCoordinates _getLeftHandSideSubset( const WindowCoordinates& coords,
                                          const QPointF& referencePoint )
{
    WindowCoordinates leftSubset;
    for( const auto& coord : coords )
    {
        const qreal centerX = coord.center().x();
        if( centerX < referencePoint.x( ))
            leftSubset.push_back( coord );
    }
    return leftSubset;
}

void _scaleAroundCenter( QRectF& rect, const qreal factor )
{
    QTransform transform;
    transform.translate( rect.center().x(), rect.center().y( ));
    transform.scale( factor, factor );
    transform.translate( -rect.center().x(), -rect.center().y( ));
    rect = transform.mapRect( rect );
}

QRectF LayoutEngine::getFocusedCoord( const ContentWindow& window ) const
{
    return _getFocusedCoord( window, _displayGroup.getFocusedWindows( ));
}

void LayoutEngine::updateFocusedCoord( ContentWindowSet& windows ) const
{
    for( auto& window : windows )
        window->setFocusedCoordinates( _getFocusedCoord( *window, windows ));
}

QRectF
LayoutEngine::_getFocusedCoord( const ContentWindow& window,
                                const ContentWindowSet& focusedWindows ) const
{
    QRectF winCoord = _getNominalCoord( window );

    if( focusedWindows.size() < 2 )
        return winCoord;

    WindowCoordinates nominalCoordinates;
    const qreal centerX = winCoord.center().x();
    for( auto win : focusedWindows )
    {
        QRectF coord = _getNominalCoord( *win );
        if( coord.center().x() == centerX )
        {
            // Use the z index to spread out overlapping windows
            coord.translate( _displayGroup.getZindex( win ), 0.0 );
            if( win->getID() == window.getID( ))
                winCoord = coord; // Update the translated coordinates
        }
        nominalCoordinates.push_back( coord );
    }

    // Compute scaling factor so that all windows fit in the available width
    const qreal totalWidth = _computeAggregatedWidth( nominalCoordinates );
    const qreal margin = _getInsideMargin();
    qreal availWidth = _displayGroup.getCoordinates().width() - 2.0 * margin;
    availWidth -= nominalCoordinates.size() * WINDOW_CONTROLS_MARGIN_PX;
    availWidth -= ( nominalCoordinates.size() - 1 ) * WINDOW_SPACING_PX;

    qreal scaleFactor = 1.0;
    qreal extraSpace = 0.0;
    if( totalWidth > availWidth )
        scaleFactor = availWidth / totalWidth;
    else
        extraSpace = ( availWidth - totalWidth ) / ( focusedWindows.size() + 1);

    // Distribute the windows horizontally so they don't overlap
    WindowCoordinates leftSubset = _getLeftHandSideSubset( nominalCoordinates,
                                                           winCoord.center( ));
    qreal leftPos = margin;
    leftPos += _computeAggregatedWidth( leftSubset ) * scaleFactor;
    leftPos += ( leftSubset.size() + 1 ) * WINDOW_CONTROLS_MARGIN_PX;
    leftPos += ( leftSubset.size() + 1 ) * extraSpace;
    leftPos += leftSubset.size() * WINDOW_SPACING_PX;

    // Scale and move the window rectangle to its final position
    _scaleAroundCenter( winCoord, scaleFactor );
    winCoord.moveLeft( leftPos );
    return winCoord;
}

QRectF LayoutEngine::_getNominalCoord( const ContentWindow& window ) const
{
    const qreal margin = 2.0 * _getInsideMargin();
    const QSizeF margins( margin + WINDOW_CONTROLS_MARGIN_PX, margin );
    const QSizeF wallSize = _displayGroup.size();
    const QSizeF maxSize = wallSize.boundedTo( wallSize - margins );

    QSizeF size = window.size();
    size.scale( maxSize, Qt::KeepAspectRatio );

    ContentWindowController( const_cast<ContentWindow&>( window ),
                             _displayGroup,
                             ContentWindowController::Coordinates::STANDARD
                             ).constrainSize( size );

    const qreal x = window.center().x();
    QRectF coord( QPointF(), size );
    coord.moveCenter( QPointF( x, wallSize.height() * 0.5 ));
    _constrainFullyInside( coord );
    return coord;
}

void LayoutEngine::_constrainFullyInside( QRectF& window ) const
{
    const QRectF& group = _displayGroup.getCoordinates();

    const qreal margin = _getInsideMargin();
    const qreal minX = margin + WINDOW_CONTROLS_MARGIN_PX;
    const qreal minY = margin;
    const qreal maxX = group.width() - window.width() - margin;
    const qreal maxY = group.height() - window.height() - margin;

    const QPointF position( std::max( minX, std::min( window.x(), maxX )),
                            std::max( minY, std::min( window.y(), maxY )));

    window.moveTopLeft( position );
}

qreal LayoutEngine::_getInsideMargin() const
{
    return _displayGroup.width() * INSIDE_MARGIN_RELATIVE +
            _displayGroup.height() * SIDEBAR_WITH_REL_TO_DISPLAYGROUP_HEIGHT;
}
