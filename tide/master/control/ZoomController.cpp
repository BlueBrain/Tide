/*********************************************************************/
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
/*                          Daniel.Nachbaur@epfl.ch                  */
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

#include "ZoomController.h"

#include "scene/Window.h"
#include "scene/ZoomHelper.h"
#include "utils/geometry.h"

namespace
{
constexpr auto MIN_ZOOM = 1.0;
}

ZoomController::ZoomController(Window& window)
    : ContentController(window)
{
    if (!window.getContent().canBeZoomed())
        throw std::invalid_argument("Content cannot be zoomed!");
}

void ZoomController::adjustZoomToContentAspectRatio()
{
    const auto zoomHelper = ZoomHelper{getWindow()};
    auto contentRect = zoomHelper.getContentRect();
    auto contentSize = QSizeF{getContent().getDimensions()};
    contentSize = contentSize / contentSize.width();
    contentSize.scale(contentRect.size(), Qt::KeepAspectRatio);
    contentRect.setSize(contentSize);

    _checkAndApply(zoomHelper.toZoomRect(contentRect));
}

void ZoomController::_doubleTap(const QPointF& position, uint numPoints)
{
    Q_UNUSED(position);
    Q_UNUSED(numPoints);
    _resetZoom();
}

void ZoomController::_tapAndHold(const QPointF& position, uint numPoints)
{
    Q_UNUSED(position);
    Q_UNUSED(numPoints);
    _resetZoom();
}

void ZoomController::_pan(const QPointF& position, const QPointF& delta,
                          const uint numPoints)
{
    Q_UNUSED(position);
    Q_UNUSED(numPoints);
    _moveZoomRect(delta);
}

void ZoomController::_pinch(const QPointF& position, const QPointF& pixelDelta)
{
    const auto zoomHelper = ZoomHelper{getWindow()};
    auto contentRect = zoomHelper.getContentRect();

    const auto expanding = pixelDelta.x() + pixelDelta.y() >= 0;
    const auto mode =
        expanding ? Qt::KeepAspectRatioByExpanding : Qt::KeepAspectRatio;

    auto newSize = contentRect.size();
    newSize.scale(newSize + QSizeF(pixelDelta.x(), pixelDelta.y()), mode);

    contentRect =
        geometry::resizeAroundPosition(contentRect, position, newSize);
    _checkAndApply(zoomHelper.toZoomRect(contentRect));

    if (!expanding && !getContent().isZoomed() && !getWindow().isFocused())
        getContent().setCaptureInteraction(false);
}

void ZoomController::_resetZoom()
{
    getContent().resetZoom();
    adjustZoomToContentAspectRatio();
}

void ZoomController::_checkAndApply(QRectF zoomRect)
{
    _constrainZoomLevel(zoomRect);
    _constraintPosition(zoomRect);
    getContent().setZoomRect(zoomRect);
}

void ZoomController::_moveZoomRect(const QPointF& sceneDelta)
{
    const auto zoomHelper = ZoomHelper{getWindow()};
    auto contentRect = zoomHelper.getContentRect();
    contentRect.translate(sceneDelta);
    _checkAndApply(zoomHelper.toZoomRect(contentRect));
}

void ZoomController::_constrainZoomLevel(QRectF& zoomRect) const
{
    const auto maxZoom = _getMaxZoom();

    // constrain max zoom
    if (zoomRect.width() < maxZoom.width() ||
        zoomRect.height() < maxZoom.height())
        zoomRect = getContent().getZoomRect();

    const auto minZoom = _getMinZoom();

    // constrain min zoom
    if (zoomRect.width() > minZoom.width())
        zoomRect.setWidth(minZoom.width());
    if (zoomRect.height() > minZoom.height())
        zoomRect.setHeight(minZoom.height());
}

void ZoomController::_constraintPosition(QRectF& zoomRect) const
{
    if (zoomRect.left() < 0.0)
        zoomRect.moveLeft(0.0);
    if (zoomRect.right() > 1.0)
        zoomRect.moveRight(1.0);
    if (zoomRect.top() < 0.0)
        zoomRect.moveTop(0.0);
    if (zoomRect.bottom() > 1.0)
        zoomRect.moveBottom(1.0);
}

QSizeF ZoomController::_getMaxZoom() const
{
    const auto window = getWindowSize();
    const auto content = getContent().getMaxUpscaledDimensions();
    return QSizeF{window.width() / content.width(),
                  window.height() / content.height()};
}

QSizeF ZoomController::_getMinZoom() const
{
    const auto window = getWindowSize();
    auto content = QSizeF(getContent().getDimensions());
    content.scale(window, Qt::KeepAspectRatioByExpanding);
    const auto windowAR = window.width() / window.height();
    const auto contentAR = content.width() / content.height();

    if (contentAR > windowAR)
        return QSizeF(MIN_ZOOM * window.width() / content.width(), MIN_ZOOM);
    return QSizeF(MIN_ZOOM, MIN_ZOOM * window.height() / content.height());
}
