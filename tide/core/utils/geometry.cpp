/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
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

#include "geometry.h"

#include "types.h"

#include <QTransform>

namespace geometry
{
QRectF resizeAroundPosition(const QRectF& rect, const QPointF& position,
                            const QSizeF& size)
{
    QTransform transform;
    transform.translate(position.x(), position.y());
    transform.scale(size.width() / rect.width(), size.height() / rect.height());
    transform.translate(-position.x(), -position.y());

    return transform.mapRect(rect);
}

QRectF scaleAroundCenter(const QRectF& rect, const qreal factor)
{
    QTransform transform;
    transform.translate(rect.center().x(), rect.center().y());
    transform.scale(factor, factor);
    transform.translate(-rect.center().x(), -rect.center().y());

    return transform.mapRect(rect);
}

QSizeF adjustAspectRatio(const QSizeF& size, const QSizeF& referenceSize)
{
    const auto mode = size < referenceSize ? Qt::KeepAspectRatio
                                           : Qt::KeepAspectRatioByExpanding;
    return referenceSize.scaled(size, mode);
}

QSizeF constrain(const QSizeF& size, const QSizeF& min, const QSizeF& max,
                 const bool keepAspectRatio)
{
    if (keepAspectRatio)
    {
        if (size < min)
            return size.scaled(min, Qt::KeepAspectRatioByExpanding);

        if (max.isValid() && size > max)
            return size.scaled(max, Qt::KeepAspectRatio);
    }
    else if (size < min || (max.isValid() && size > max))
    {
        return QSizeF{std::max(min.width(),
                               std::min(size.width(), max.width())),
                      std::max(min.height(),
                               std::min(size.height(), max.height()))};
    }
    return size;
}
}
