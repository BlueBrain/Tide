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

#include "ZoomHelper.h"

#include "scene/ContentWindow.h"

ZoomHelper::ZoomHelper(const ContentWindow& window)
    : _contentWindow(window)
{
}

QRectF ZoomHelper::getContentRect() const
{
    return toContentRect(_contentWindow.getContent()->getZoomRect());
}

QRectF ZoomHelper::toContentRect(const QRectF& zoomRect) const
{
    const QRectF& window = _contentWindow.getDisplayCoordinates();

    const qreal w = window.width() / zoomRect.width();
    const qreal h = window.height() / zoomRect.height();

    const qreal posX = -zoomRect.x() * w;
    const qreal posY = -zoomRect.y() * h;

    return QRectF(posX, posY, w, h);
}

QRectF ZoomHelper::toZoomRect(const QRectF& contentRect) const
{
    const QRectF& window = _contentWindow.getDisplayCoordinates();

    const qreal w = window.width() / contentRect.width();
    const qreal h = window.height() / contentRect.height();

    const qreal posX = -contentRect.x() / contentRect.width();
    const qreal posY = -contentRect.y() / contentRect.height();

    return QRectF(posX, posY, w, h);
}

QRectF ZoomHelper::toTilesArea(const QRectF& windowArea,
                               const QSize& tilesSurface) const
{
    const QRectF contentRect = getContentRect();

    // Map window visibleArea to content space for tiles origin at (0,0)
    const QRectF visibleContentArea =
        windowArea.translated(-contentRect.x(), -contentRect.y());
    // Scale content area to tiles area size
    const qreal xScale = tilesSurface.width() / contentRect.width();
    const qreal yScale = tilesSurface.height() / contentRect.height();
    const QRectF visibleTilesArea(visibleContentArea.x() * xScale,
                                  visibleContentArea.y() * yScale,
                                  visibleContentArea.width() * xScale,
                                  visibleContentArea.height() * yScale);
    return visibleTilesArea;
}
