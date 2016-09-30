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

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <QRectF>

/**
 * Set of geometry functions.
 */
namespace geometry
{

/**
 * Get the size of a surface scaled to fit inside another one
 * @param source the source size
 * @param target the target size
 * @return the adjusted size
 */
template <typename Source, typename Target>
inline QSizeF getAdjustedSize( const Source& source, const Target& target )
{
    auto size = QSizeF( source.width(), source.height( ));
    return size.scaled( target.size(), Qt::KeepAspectRatio );
}

/**
 * Get the size of a surface scaled to fit around another one
 * @param source the source size
 * @param target the target size
 * @return the adjusted size
 */
template <typename Source, typename Target>
inline QSizeF getExpandedSize( const Source& source, const Target& target )
{
    auto size = QSizeF( source.width(), source.height( ));
    return size.scaled( target.size(), Qt::KeepAspectRatioByExpanding );
}

/**
 * Adjust a surface to exactly fit inside another one, preserving aspect ratio
 * @param source the source surface
 * @param target the destination surface
 * @return the adjusted rectangle
 */
template <typename Source, typename Target>
QRectF adjustAndCenter( const Source& source, const Target& target )
{
    auto rect = QRectF{ QPointF(), getAdjustedSize( source, target ) };
    rect.moveCenter( target.center( ));
    return rect;
}

/**
 * Resize a rectangle around a point of interest.
 *
 * @param rect the rectangle to resize
 * @param position the point of interest to resize around
 * @param size the new absolute size to resize to
 * @return the resized rectangle
 */
QRectF resizeAroundPosition( const QRectF& rect, const QPointF& position,
                             const QSizeF& size );

/**
 * Constrain a size between min and max values.
 * @param size the size to constrain
 * @param min the minimum size (ignored if not valid)
 * @param max the maximum size (ignored if not valid)
 * @return the size comprised between min and max
 */
QSizeF constrain( const QSizeF& size, const QSizeF& min, const QSizeF& max );

}

#endif
