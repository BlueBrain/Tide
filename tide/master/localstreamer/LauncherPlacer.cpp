/*********************************************************************/
/* Copyright (c) 2018, EPFL/Blue Brain Project                       */
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

#include "LauncherPlacer.h"

#include "configuration/SurfaceConfig.h"
#include "utils/geometry.h"

namespace
{
const auto dimensionsInMeters = QSizeF{0.8, 0.72};
const auto aspectRatio =
    dimensionsInMeters.width() / dimensionsInMeters.height();
const auto minSizePx = QSize{256, 256};
const auto maxSizePx = QSize{1920, 1920};
const auto defaultRelHeight = 0.5;
const auto maxRelSize = 0.9;
const auto minRelTopMargin = (1.0 - maxRelSize) * 0.5;
}

LauncherPlacer::LauncherPlacer(const SurfaceConfig& surface)
    : _surface(surface)
{
}

QRect LauncherPlacer::getCoordinates() const
{
    auto rect = _getCoordinatesForStandardWall();

    if (_launcherIsTooHigh(rect))
        _centerVertically(rect);

    return rect.toRect();
}

QSize LauncherPlacer::_getSize() const
{
    auto size = _surface.toPixelSize(dimensionsInMeters);
    if (size.isEmpty())
        size = _getDefaultSizePx();

    return geometry::constrain(size, minSizePx, _getMaxSizePx()).toSize();
}

QSize LauncherPlacer::_getDefaultSizePx() const
{
    const auto height = defaultRelHeight * _surface.getTotalHeight();
    return QSize(height * aspectRatio, height);
}

QSize LauncherPlacer::_getMaxSizePx() const
{
    const auto maxSize = maxRelSize * _surface.getTotalSize();
    if (maxSize.width() < maxSizePx.width())
        return maxSize;
    return maxSizePx;
}

QRectF LauncherPlacer::_getCoordinatesForStandardWall() const
{
    auto rect = QRectF{QPointF(), _getSize()};
    _centerAboveMiddleOfSurface(rect);
    return rect;
}

bool LauncherPlacer::_launcherIsTooHigh(const QRectF& rect) const
{
    return _exceedsTopMargin(rect) || _surfaceLikelyDoesNotStartFromTheFloor();
}

bool LauncherPlacer::_exceedsTopMargin(const QRectF& rect) const
{
    return rect.top() < minRelTopMargin * _surface.getTotalHeight();
}

bool LauncherPlacer::_surfaceLikelyDoesNotStartFromTheFloor() const
{
    return _surface.dimensions.isValid() && _surface.dimensions.height() < 1.6;
}

void LauncherPlacer::_centerAboveMiddleOfSurface(QRectF& rect) const
{
    rect.moveCenter({0.9 * rect.width(), 0.35 * _surface.getTotalHeight()});
}

void LauncherPlacer::_centerVertically(QRectF& rect) const
{
    rect.moveCenter({0.9 * rect.width(), 0.5 * _surface.getTotalHeight()});
}
