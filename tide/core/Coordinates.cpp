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

#include "Coordinates.h"

Coordinates::Coordinates()
{
}

Coordinates::~Coordinates()
{
}

// false-positive on qt signals
// cppcheck-suppress uninitMemberVar
Coordinates::Coordinates( const QRectF& coordinates )
    : _coordinates( coordinates )
{
}

const QRectF& Coordinates::getCoordinates() const
{
    return _coordinates;
}

qreal Coordinates::x() const
{
    return _coordinates.x();
}

qreal Coordinates::y() const
{
    return _coordinates.y();
}

qreal Coordinates::width() const
{
    return _coordinates.width();
}

qreal Coordinates::height() const
{
    return _coordinates.height();
}

void Coordinates::setX( const qreal x_ )
{
    if( x_ == _coordinates.x( ))
        return;

    _coordinates.setX( x_ );
    emit xChanged();
}

void Coordinates::setY( const qreal y_ )
{
    if( y_ == _coordinates.y( ))
        return;

    _coordinates.setY( y_ );
    emit yChanged();
}

void Coordinates::setWidth( const qreal w )
{
    if( w == _coordinates.width( ))
        return;

    _coordinates.setWidth( w );
    emit widthChanged();
}

void Coordinates::setHeight( const qreal h )
{
    if( h == _coordinates.height( ))
        return;

    _coordinates.setHeight( h );
    emit heightChanged();
}

void Coordinates::setCoordinates( const QRectF& coordinates )
{
    setX( coordinates.x( ));
    setY( coordinates.y( ));
    setWidth( coordinates.width( ));
    setHeight( coordinates.height( ));
}
