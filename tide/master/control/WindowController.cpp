/*********************************************************************/
/* Copyright (c) 2014-2018, EPFL/Blue Brain Project                  */
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

#include "WindowController.h"

#include "ZoomController.h"
#include "scene/DisplayGroup.h"
#include "scene/ZoomHelper.h"
#include "ui.h"
#include "utils/geometry.h"

#include <cmath>

namespace
{
constexpr qreal FITTING_SIZE_SCALE = 0.9;
constexpr qreal ONE_PERCENT = 0.01;
constexpr auto INVALID_CONTROLLER_MSG =
    "ContentController MUST be of class type ZoomController when "
    "content.canBeZoomed() is true";

Window::ResizePolicy _getResizePolicyForAjustingSize(const Content& content)
{
    return content.hasFixedAspectRatio() || content.canBeZoomed()
               ? Window::ResizePolicy::KEEP_ASPECT_RATIO
               : Window::ResizePolicy::ADJUST_CONTENT;
}
}

WindowController::WindowController(Window& window, const DisplayGroup& group,
                                   const Coordinates target)
    : _window{window}
    , _group{group}
    , _target{target}
{
}

void WindowController::resize(const QSizeF& size, const WindowPoint fixedPoint,
                              const Window::ResizePolicy policy)
{
    switch (fixedPoint)
    {
    case CENTER:
        resize(_getCoordinates().center(), size, policy);
        break;
    case TOP_LEFT:
        resize(_getCoordinates().topLeft(), size, policy);
    }
}

void WindowController::resize(const QPointF& center, QSizeF newSize,
                              const Window::ResizePolicy policy)
{
    constrainSize(newSize, policy);

    auto coordinates = _getCoordinates();
    coordinates = geometry::resizeAroundPosition(coordinates, center, newSize);
    _constrainPosition(coordinates);

    _apply(coordinates);

    if (_contentZoomCanBeAdjusted())
        _adjustZoom();
}

void WindowController::scale(const QPointF& center, const double pixelDelta)
{
    auto newSize = _getSize();
    newSize.scale(newSize.width() + pixelDelta, newSize.height() + pixelDelta,
                  pixelDelta < 0 ? Qt::KeepAspectRatio
                                 : Qt::KeepAspectRatioByExpanding);
    resize(center, newSize);
}

void WindowController::scale(const QPointF& center, const QPointF& pixelDelta)
{
    const auto sign = pixelDelta.x() + pixelDelta.y() > 0.0 ? 1.0 : -1.0;
    const auto delta = std::sqrt(pixelDelta.x() * pixelDelta.x() +
                                 pixelDelta.y() * pixelDelta.y());
    scale(center, sign * delta);
}

void WindowController::adjustSize(const SizeState state)
{
    const auto policy = _getResizePolicyForAjustingSize(_window.getContent());

    switch (state)
    {
    case SIZE_1TO1:
        resize(_getPreferredDimensions(), CENTER, policy);
        break;

    case SIZE_1TO1_FITTING:
    {
        const auto oneToOneSize = _getPreferredDimensions();
        const auto maxSize = _group.size() * FITTING_SIZE_SCALE;
        const auto size = geometry::constrain(oneToOneSize, QSizeF(), maxSize);
        resize(size, CENTER, policy);
    }
    break;

    case SIZE_FULLSCREEN:
    {
        _window.getContent().resetZoom();

        auto size =
            geometry::getAdjustedSize(_getPreferredDimensions(), _group);
        constrainSize(size, policy);
        size = geometry::constrain(size, QSizeF(), _group.size());
        _apply(_getCenteredCoordinates(size));
    }
    break;

    case SIZE_FULLSCREEN_MAX:
    {
        _window.getContent().resetZoom();

        auto size = geometry::getExpandedSize(_window.getContent(), _group);
        constrainSize(size, policy);
        _apply(_getCenteredCoordinates(size));
    }
    break;

    case SIZE_FULLSCREEN_1TO1:
    {
        _window.getContent().resetZoom();

        auto size = _getPreferredDimensions();
        constrainSize(size, policy);
        _apply(_getCenteredCoordinates(size));
    }
    break;
    }
}

void WindowController::toogleFullscreenMaxSize()
{
    if (!_targetIsFullscreen())
        return;

    if (_getSize() > _group.size())
        adjustSize(SizeState::SIZE_FULLSCREEN);
    else
        adjustSize(SizeState::SIZE_FULLSCREEN_MAX);
}

void WindowController::toggleSelected()
{
    _window.setSelected(!_window.isSelected());
}

void WindowController::moveTo(const QPointF& position, const WindowPoint handle)
{
    auto coordinates = _getCoordinates();
    switch (handle)
    {
    case TOP_LEFT:
        coordinates.moveTopLeft(position);
        break;
    case CENTER:
        coordinates.moveCenter(position);
        break;
    }
    _constrainPosition(coordinates);

    _apply(coordinates);
}

void WindowController::moveBy(const QPointF& delta)
{
    moveTo(_getCoordinates().topLeft() + delta);
}

QSizeF WindowController::getMinSize() const
{
    if (_targetIsFullscreen())
    {
        const auto size = _getPreferredDimensions();
        const auto targetSize = size.scaled(_group.size(), Qt::KeepAspectRatio);
        return std::min(targetSize, getMaxSize());
    }

    const auto minContentSize = QSizeF{_window.getContent().getMinDimensions()};
    const auto minSize = QSizeF{ui::getMinWindowSize(), ui::getMinWindowSize()};
    return std::max(minContentSize, minSize);
}

QSizeF WindowController::getMaxSize() const
{
    return ZoomHelper{_window}.getMaxWindowSizeUpscaled();
}

QSizeF WindowController::getMinSizeAspectRatioCorrect() const
{
    const auto min = getMinSize();
    const auto max = getMaxSize();
    if (min > max)
        return _getAspectRatioSize().scaled(max, Qt::KeepAspectRatio);
    return _getAspectRatioSize().scaled(min, Qt::KeepAspectRatioByExpanding);
}

void WindowController::constrainSize(QSizeF& windowSize,
                                     const Window::ResizePolicy policy) const
{
    const auto snapToAspectRatio =
        policy == Window::KEEP_ASPECT_RATIO && _window.getContent().isZoomed();

    const auto tryKeepAspectRatio =
        policy == Window::KEEP_ASPECT_RATIO && !_window.getContent().isZoomed();

    const auto keepAspectRatio =
        tryKeepAspectRatio || _mustKeepAspectRatio(windowSize);

    if (keepAspectRatio)
        _constrainAspectRatio(windowSize);

    windowSize = geometry::constrain(windowSize, getMinSize(), getMaxSize(),
                                     keepAspectRatio);

    if (snapToAspectRatio && _isCloseToContentAspectRatio(windowSize))
        windowSize = geometry::adjustAspectRatio(windowSize, _getContentSize());
}

void WindowController::_constrainAspectRatio(QSizeF& newSize) const
{
    // Warning: using the current size to decide on scaling mode is needed for
    // interactive resize() with handles but does not make sense on adjust().
    const auto currentSize = _window.getDisplayCoordinates().size();
    const auto mode = newSize < currentSize ? Qt::KeepAspectRatio
                                            : Qt::KeepAspectRatioByExpanding;
    newSize = _getAspectRatioSize().scaled(newSize, mode);
}

void WindowController::_constrainPosition(QRectF& window) const
{
    const auto& group = _group.getCoordinates();

    if (_targetIsFullscreen())
    {
        const auto overlapX = group.width() - window.width();
        const auto overlapY = group.height() - window.height();

        const auto minX = overlapX < 0.0 ? overlapX : 0.5 * overlapX;
        const auto minY = overlapY < 0.0 ? overlapY : 0.5 * overlapY;

        const auto maxX = 0.0;
        const auto maxY = 0.0;

        window.moveTopLeft({std::max(minX, std::min(window.x(), maxX)),
                            std::max(minY, std::min(window.y(), maxY))});
        return;
    }

    const auto minX = ui::getMinWindowSize() - window.width();
    const auto minY = ui::getMinWindowSize() - window.height();

    const auto maxX = group.width() - ui::getMinWindowSize();
    const auto maxY = group.height() - ui::getMinWindowSize();

    window.moveTopLeft({std::max(minX, std::min(window.x(), maxX)),
                        std::max(minY, std::min(window.y(), maxY))});
}

void WindowController::_adjustZoom() const
{
    auto controller = ContentController::create(_window);
    auto zoomController = dynamic_cast<ZoomController*>(controller.get());
    if (!zoomController)
        throw std::logic_error(INVALID_CONTROLLER_MSG);
    zoomController->adjustZoomToContentAspectRatio();
}

bool WindowController::_mustKeepAspectRatio(const QSizeF& newSize) const
{
    return !_contentZoomCanBeAdjusted() && !_contentSourceCanBeResized(newSize);
}

bool WindowController::_contentZoomCanBeAdjusted() const
{
    return _window.getContent().canBeZoomed();
}

bool WindowController::_contentSourceCanBeResized(const QSizeF& newSize) const
{
    return !_window.getContent().hasFixedAspectRatio() &&
           !(newSize >=
             ZoomHelper{_window}.getMaxWindowSizeAtNativeResolution());
}

bool WindowController::_isCloseToContentAspectRatio(
    const QSizeF& windowSize) const
{
    const auto windowAR = windowSize.width() / windowSize.height();
    return std::fabs(windowAR - _getContentAspectRatio()) < ONE_PERCENT;
}

bool WindowController::_targetIsFullscreen() const
{
    return _target == Coordinates::FULLSCREEN ||
           (_target == Coordinates::AUTO && _window.isFullscreen());
}

QSizeF WindowController::_getAspectRatioSize() const
{
    return QSizeF{_getContentAspectRatio(), 1.0};
}

qreal WindowController::_getContentAspectRatio() const
{
    return _window.getContent().getAspectRatio();
}

QSizeF WindowController::_getPreferredDimensions() const
{
    return _window.getContent().getPreferredDimensions();
}

QRectF WindowController::_getCenteredCoordinates(const QSizeF& size) const
{
    auto coord = QRectF{QPointF(), size};
    coord.moveCenter(_group.getCoordinates().center());
    return coord;
}

const QRectF& WindowController::_getCoordinates() const
{
    switch (_target)
    {
    case Coordinates::STANDARD:
        return _window.getCoordinates();
    case Coordinates::FOCUSED:
        return _window.getFocusedCoordinates();
    case Coordinates::FULLSCREEN:
        return _window.getFullscreenCoordinates();
    case Coordinates::AUTO:
        return _window.getDisplayCoordinates();
    }
    throw std::logic_error("invalid _target");
}

QSizeF WindowController::_getSize() const
{
    return _getCoordinates().size();
}

QSizeF WindowController::_getContentSize() const
{
    return _window.getContent().getDimensions();
}

void WindowController::_apply(const QRectF& coordinates)
{
    switch (_target)
    {
    case Coordinates::STANDARD:
        _window.setCoordinates(coordinates);
        break;
    case Coordinates::FOCUSED:
        _window.setFocusedCoordinates(coordinates);
        break;
    case Coordinates::FULLSCREEN:
        _window.setFullscreenCoordinates(coordinates);
        break;
    case Coordinates::AUTO:
        _window.setDisplayCoordinates(coordinates);
        break;
    }
}
