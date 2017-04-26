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

#include "control/ContentWindowController.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"

#include <QTransform>

namespace
{
const qreal INSIDE_MARGIN_RELATIVE = 0.02;
const qreal SIDEBAR_WITH_REL_TO_DISPLAYGROUP_HEIGHT = 0.3 * 0.3;
const qreal WINDOW_CONTROLS_MARGIN_PX = 200.0;
const qreal WINDOW_SPACING_PX = 80.0;
}

struct WindowCoordinates
{
    const QRectF rect;
    const int z;
};
using WindowList = std::vector<WindowCoordinates>;

LayoutEngine::LayoutEngine(const DisplayGroup& group)
    : _group(group)
{
}

qreal _computeAggregatedWidth(const WindowList& windows)
{
    qreal width = 0.0;
    for (const auto& win : windows)
        width += win.rect.width();
    return width;
}

WindowList _getLeftHandSideSubset(const WindowList& windows,
                                  const WindowCoordinates& reference)
{
    WindowList leftSubset;
    const auto refX = reference.rect.center().x();
    for (const auto& win : windows)
    {
        const auto winX = win.rect.center().x();
        // Use z order to disambiguate overlapping windows
        if (winX < refX || (winX == refX && win.z < reference.z))
            leftSubset.push_back(win);
    }
    return leftSubset;
}

QRectF _scaleAroundCenter(const QRectF& rect, const qreal factor)
{
    QTransform transform;
    transform.translate(rect.center().x(), rect.center().y());
    transform.scale(factor, factor);
    transform.translate(-rect.center().x(), -rect.center().y());
    return transform.mapRect(rect);
}

QRectF LayoutEngine::getFocusedCoord(const ContentWindow& window) const
{
    return _getFocusedCoord(window, _group.getFocusedWindows());
}

void LayoutEngine::updateFocusedCoord(const ContentWindowSet& windows) const
{
    for (auto& window : windows)
        window->setFocusedCoordinates(_getFocusedCoord(*window, windows));
}

QRectF LayoutEngine::_getFocusedCoord(
    const ContentWindow& window, const ContentWindowSet& focusedWindows) const
{
    if (focusedWindows.size() < 2)
        return _getNominalCoord(window).rect;

    WindowList nominalCoords;
    nominalCoords.reserve(focusedWindows.size());
    for (const auto& win : focusedWindows)
        nominalCoords.emplace_back(_getNominalCoord(*win));

    // Compute scaling factor so that all windows fit in the available width
    const qreal totalWidth = _computeAggregatedWidth(nominalCoords);
    const qreal insideMargin = _getInsideMargin();
    qreal availWidth = _group.width() - 2.0 * insideMargin;
    availWidth -= focusedWindows.size() * WINDOW_CONTROLS_MARGIN_PX;
    availWidth -= (focusedWindows.size() - 1) * WINDOW_SPACING_PX;

    qreal scaleFactor = 1.0;
    qreal extraSpace = 0.0;
    if (totalWidth > availWidth)
        scaleFactor = availWidth / totalWidth;
    else
        extraSpace = (availWidth - totalWidth) / (focusedWindows.size() + 1);

    // Distribute the windows horizontally so they don't overlap
    const auto winCoord = _getNominalCoord(window);
    const auto leftSubset = _getLeftHandSideSubset(nominalCoords, winCoord);
    qreal leftPos = insideMargin;
    leftPos += _computeAggregatedWidth(leftSubset) * scaleFactor;
    leftPos += (leftSubset.size() + 1) * WINDOW_CONTROLS_MARGIN_PX;
    leftPos += (leftSubset.size() + 1) * extraSpace;
    leftPos += leftSubset.size() * WINDOW_SPACING_PX;

    // Scale and move the window rectangle to its final position
    auto coord = _scaleAroundCenter(winCoord.rect, scaleFactor);
    coord.moveLeft(leftPos);
    return coord;
}

WindowCoordinates LayoutEngine::_getNominalCoord(
    const ContentWindow& window) const
{
    const qreal margin = 2.0 * _getInsideMargin();
    const QSizeF margins(margin + WINDOW_CONTROLS_MARGIN_PX, margin);
    const QSizeF wallSize = _group.size();
    const QSizeF maxSize = wallSize.boundedTo(wallSize - margins);

    QSizeF size = window.size();
    size.scale(maxSize, Qt::KeepAspectRatio);

    ContentWindowController(const_cast<ContentWindow&>(window), _group,
                            ContentWindowController::Coordinates::STANDARD)
        .constrainSize(size);

    QRectF coord(QPointF(), size);
    coord.moveCenter(QPointF(window.center().x(), wallSize.height() * 0.5));
    _constrainFullyInside(coord);
    auto winPtr = _group.getContentWindow(window.getID());
    return {coord, _group.getZindex(winPtr)};
}

void LayoutEngine::_constrainFullyInside(QRectF& window) const
{
    const qreal margin = _getInsideMargin();
    const qreal minX = margin + WINDOW_CONTROLS_MARGIN_PX;
    const qreal minY = margin;
    const qreal maxX = _group.width() - window.width() - margin;
    const qreal maxY = _group.height() - window.height() - margin;

    const QPointF position(std::max(minX, std::min(window.x(), maxX)),
                           std::max(minY, std::min(window.y(), maxY)));

    window.moveTopLeft(position);
}

qreal LayoutEngine::_getInsideMargin() const
{
    return _group.width() * INSIDE_MARGIN_RELATIVE +
           _group.height() * SIDEBAR_WITH_REL_TO_DISPLAYGROUP_HEIGHT;
}
