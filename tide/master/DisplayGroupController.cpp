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

#include "DisplayGroup.h"

#include <QTransform>

DisplayGroupController::DisplayGroupController( DisplayGroup& group )
    : _group( group )
{}

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
    QSizeF targetSize = _group.getCoordinates().size();
    targetSize.scale( maxGroupSize, Qt::KeepAspectRatio );
    const qreal scaleFactor = targetSize.width() / _group.width();
    scale( QSizeF( scaleFactor, scaleFactor ));
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
