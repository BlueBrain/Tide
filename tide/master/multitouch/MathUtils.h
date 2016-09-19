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
#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <QRectF>

#include "types.h"

/**
 * Utility math functions used by multitouch gesture detectors.
 */
namespace MathUtils
{

/**
 * Get the minimal bounding rectangle around two points.
 * @param p0 the first corner
 * @param p1 the second corner
 * @return the bounding rectangle
 */
QRectF getBoundingRect( const QPointF& p0, const QPointF& p1 );

/**
 * Get the euclidean distance between two points.
 * @param p0 the first point
 * @param p1 the second point
 * @return the distance between the two points.
 */
qreal getDist( const QPointF& p0, const QPointF& p1 );

/**
 * Get the center of two points.
 * @param p0 the first point
 * @param p1 the second point
 * @return the center of the two points.
 */
QPointF getCenter( const QPointF& p0, const QPointF& p1 );

/**
 * Compute the center of a list of positions.
 * @return the center of the points.
 */
QPointF computeCenter( const Positions& positions );

/**
 * Check if any point has moved by more than a given threshold.
 * @param positions the current positions
 * @param startPositions the start positions
 * @param moveThreshold the distance threshold
 * @return true if any point has moved
 */
bool hasMoved( const Positions& positions, const Positions& startPositions,
               qreal moveThreshold );

}

#endif
