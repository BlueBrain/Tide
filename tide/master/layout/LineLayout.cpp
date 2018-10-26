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

#include "LineLayout.h"

#include "control/WindowController.h"
#include "scene/DisplayGroup.h"
#include "scene/Window.h"
#include "ui.h"
#include "utils/geometry.h"

struct WindowCoordinates
{
    const QRectF rect;
    const int z;
};
using WindowList = std::vector<WindowCoordinates>;

namespace
{
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
}

LineLayout::LineLayout(const DisplayGroup& group)
    : _group(group)
{
}

void LineLayout::updateFocusedCoord(const WindowSet& windows) const
{
    for (auto& window : windows)
        window->setFocusedCoordinates(_getFocusedCoord(*window, windows));
}

QRectF LineLayout::getFocusedCoord(const Window& window) const
{
    return _getFocusedCoord(window, _group.getFocusedWindows());
}

QRectF LineLayout::_getFocusedCoord(const Window& window,
                                    const WindowSet& focusedWindows) const
{
    if (focusedWindows.size() < 2)
        return _getNominalCoord(window).rect;

    WindowList nominalCoords;
    nominalCoords.reserve(focusedWindows.size());
    for (const auto& win : focusedWindows)
        nominalCoords.emplace_back(_getNominalCoord(*win));

    const auto surface = ui::getFocusSurface(_group);

    // Compute scaling factor so that all windows fit in the available width
    const qreal totalWidth = _computeAggregatedWidth(nominalCoords);
    auto availWidth = surface.width();
    availWidth -= focusedWindows.size() * ui::getWindowControlsMargin();
    availWidth -= (focusedWindows.size() - 1) * ui::getMinWindowSpacing();

    qreal scaleFactor = 1.0;
    qreal extraSpace = 0.0;
    if (totalWidth > availWidth)
        scaleFactor = availWidth / totalWidth;
    else
        extraSpace = (availWidth - totalWidth) / (focusedWindows.size() + 1);

    // Distribute the windows horizontally so they don't overlap
    const auto winCoord = _getNominalCoord(window);
    const auto leftSubset = _getLeftHandSideSubset(nominalCoords, winCoord);
    auto leftPos = surface.x();
    leftPos += _computeAggregatedWidth(leftSubset) * scaleFactor;
    leftPos += (leftSubset.size() + 1) * ui::getWindowControlsMargin();
    leftPos += (leftSubset.size() + 1) * extraSpace;
    leftPos += leftSubset.size() * ui::getMinWindowSpacing();

    // Scale and move the window rectangle to its final position
    auto coord = geometry::scaleAroundCenter(winCoord.rect, scaleFactor);
    coord.moveLeft(leftPos);
    return coord;
}

WindowCoordinates LineLayout::_getNominalCoord(const Window& window) const
{
    const auto surface = ui::getFocusSurface(_group);
    const auto maxSize =
        surface.size() - QSizeF(ui::getWindowControlsMargin(), 0);
    auto size = window.size().scaled(maxSize, Qt::KeepAspectRatio);

    WindowController(const_cast<Window&>(window), _group,
                     WindowController::Coordinates::STANDARD)
        .constrainSize(size, Window::ResizePolicy::KEEP_ASPECT_RATIO);

    auto coord = QRectF(QPointF(), size);
    coord.moveCenter(QPointF(window.center().x(), surface.center().y()));
    _constrainFullyInside(coord);
    return {coord, _group.getZindex(window.getID())};
}

void LineLayout::_constrainFullyInside(QRectF& window) const
{
    const auto surface = ui::getFocusSurface(_group);

    const qreal minX = surface.x() + ui::getWindowControlsMargin();
    const qreal minY = surface.y();
    const qreal maxX = surface.right() - window.width();
    const qreal maxY = surface.bottom() - window.height();

    const QPointF position(std::max(minX, std::min(window.x(), maxX)),
                           std::max(minY, std::min(window.y(), maxY)));

    window.moveTopLeft(position);
}
