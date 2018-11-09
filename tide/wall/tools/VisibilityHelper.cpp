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
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#include "VisibilityHelper.h"

#include "scene/DisplayGroup.h"
#include "scene/Window.h"

VisibilityHelper::VisibilityHelper(const DisplayGroup& displayGroup,
                                   const QRect& visibleArea,
                                   const bool alphaBlending)
    : _displayGroup{displayGroup}
    , _visibleArea{visibleArea}
    , _alphaBlending{alphaBlending}
{
}

QRectF _cutOverlap(const QRectF& window, const QRectF& other)
{
    if (other.contains(window))
        return QRectF();

    const QRectF overlap = window.intersected(other);
    if (overlap.isEmpty())
        return window;

    // Full horizontal cut
    if (overlap.width() >= window.width())
    {
        QRectF area(window);
        if (overlap.top() <= window.top())
            area.setTop(overlap.bottom());
        else if (overlap.bottom() >= window.bottom())
            area.setBottom(overlap.top());
        return area;
    }

    // Full vertical cut
    if (overlap.height() >= window.height())
    {
        QRectF area(window);
        if (overlap.left() <= window.left())
            area.setLeft(overlap.right());
        else if (overlap.right() >= window.right())
            area.setRight(overlap.left());
        return area;
    }

    return window;
}

QRectF _globalToWindowCoordinates(const QRectF& area, const QRectF& window)
{
    return area.translated(-window.x(), -window.y());
}

QRectF VisibilityHelper::getVisibleArea(const Window& window) const
{
    const auto& windowCoords = window.getDisplayCoordinates();

    auto area = windowCoords.intersected(_visibleArea);
    if (area.isEmpty())
        return QRectF();

    if (window.isFullscreen())
        return _globalToWindowCoordinates(area, windowCoords);

    if (_displayGroup.hasFullscreenWindows())
        return QRectF();

    if (window.isFocused())
        return _globalToWindowCoordinates(area, windowCoords);

    auto winIsAbove = false;
    for (const auto& win : _displayGroup.getWindows())
    {
        if (win->getID() == window.getID())
        {
            winIsAbove = !window.isPanel(); // panels are above regular windows
            continue;
        }
        if (win->isHidden())
            continue;

        if ((winIsAbove || win->isFocused()) &&
            (!_alphaBlending || !win->getContent().hasTransparency()))
        {
            area = _cutOverlap(area, win->getDisplayCoordinates());
        }

        if (area.isEmpty())
            return QRectF();
    }

    return _globalToWindowCoordinates(area, windowCoords);
}
