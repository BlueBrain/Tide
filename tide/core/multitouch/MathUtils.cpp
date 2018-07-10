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

#include "MathUtils.h"

#include <cmath>

namespace MathUtils
{
QRectF getBoundingRect(const QPointF& p0, const QPointF& p1)
{
    QRectF rect;
    rect.setLeft(std::min(p0.x(), p1.x()));
    rect.setRight(std::max(p0.x(), p1.x()));
    rect.setTop(std::min(p0.y(), p1.y()));
    rect.setBottom(std::max(p0.y(), p1.y()));
    return rect;
}

qreal getDist(const QPointF& p0, const QPointF& p1)
{
    const QPointF dist = p1 - p0;
    return std::sqrt(QPointF::dotProduct(dist, dist));
}

QPointF getCenter(const QPointF& p0, const QPointF& p1)
{
    return (p0 + p1) / 2;
}

QPointF computeCenter(const Positions& positions)
{
    QPointF center;
    for (const auto& pos : positions)
        center += pos;
    return center / positions.size();
}

bool hasMoved(const Positions& positions, const Positions& startPositions,
              const qreal moveThreshold)
{
    size_t i = 0;
    for (const auto& pos : positions)
    {
        if ((pos - startPositions[i++]).manhattanLength() > moveThreshold)
            return true;
    }
    return false;
}
}
