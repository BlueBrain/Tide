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

#include "DisplayGroupController.h"

#include "WindowController.h"
#include "layout/LayoutEngine.h"
#include "scene/DisplayGroup.h"
#include "scene/Window.h"

#include <QTransform>

namespace
{
static auto _exceeds = [](const auto& size, const auto& other) {
    return size.width() > other.width() && size.height() > other.height();
};
}

DisplayGroupController::DisplayGroupController(DisplayGroup& group)
    : _group(group)
{
}

void DisplayGroupController::remove(const QUuid windowId)
{
    auto window = _group.getWindow(windowId);
    if (!window)
        return;

    const auto focused = window->isFocused();
    _group.remove(window);
    if (focused)
        updateFocusedWindowsCoordinates();
}

void DisplayGroupController::removeLater(const QUuid windowId)
{
    QMetaObject::invokeMethod(this, "remove", Qt::QueuedConnection,
                              Q_ARG(QUuid, windowId));
}

bool DisplayGroupController::showFullscreen(const QUuid& id)
{
    auto window = _group.getWindow(id);
    if (!window || window->isPanel())
        return false;

    _showFullscreen(window, false);
    return true;
}

void DisplayGroupController::exitFullscreen()
{
    auto window = _group.getFullscreenWindow();
    if (!window)
        return;

    window->restoreModeAndZoom();
    _group.setFullscreenWindow(WindowPtr());

    if (!window->isFocused())
        window->setSelected(false);
}

bool DisplayGroupController::toggleFullscreen(const QUuid& id)
{
    if (auto window = _group.getFullscreenWindow())
    {
        if (window->getID() == id)
        {
            exitFullscreen();
            return true;
        }
    }
    return showFullscreen(id);
}

void DisplayGroupController::adjustSizeOneToOne(const QUuid& id)
{
    auto window = _group.getWindow(id);
    if (!window)
        return;

    if (_exceeds(window->getContent().getPreferredDimensions(), _group))
        _showFullscreen(window, true);
    else
    {
        WindowController{*window, _group}.adjustSize(SIZE_1TO1);
        window->getContent().resetZoom();
    }
}

bool DisplayGroupController::focus(const QUuid& id)
{
    auto window = _group.getWindow(id);
    if (!window || window->isPanel() ||
        _group.getFocusedWindows().count(window))
        return false;

    // Update coordinates BEFORE focusing window for proper transition
    auto focusedWindows = _group.getFocusedWindows();
    focusedWindows.insert(window);
    _updateFocusedCoordinates(focusedWindows);

    _group.addFocusedWindow(window);
    return true;
}

bool DisplayGroupController::unfocus(const QUuid& id)
{
    auto window = _group.getWindow(id);
    if (!window || !_group.getFocusedWindows().count(window))
        return false;

    window->setSelected(false);
    _group.removeFocusedWindow(window);
    _readjustToNewZoomLevel(*window);

    updateFocusedWindowsCoordinates();
    return true;
}

void DisplayGroupController::focusSelected()
{
    _focus(_group.getSelectedWindows());
}

void DisplayGroupController::focusAll()
{
    _focus(_group.getFocusableWindows());
}

void DisplayGroupController::unfocusAll()
{
    const auto focusedWindows = _group.getFocusedWindows();

    for (const auto& window : focusedWindows)
    {
        _group.removeFocusedWindow(window);
        _readjustToNewZoomLevel(*window);
    }

    updateFocusedWindowsCoordinates();
}

void DisplayGroupController::toggleFocusAll()
{
    if (_group.hasFocusedWindows())
        unfocusAll();
    else
        focusAll();
}

void DisplayGroupController::deselectAll()
{
    for (auto&& window : _group.getWindows())
        window->setSelected(false);
}

void DisplayGroupController::hidePanels()
{
    for (const auto& panel : _group.getPanels())
        panel->setState(Window::HIDDEN);
}

bool DisplayGroupController::moveWindowToFront(const QUuid& id)
{
    const auto window = _group.getWindow(id);
    if (!window)
        return false;
    if (window->getMode() == Window::WindowMode::STANDARD)
        _group.moveToFront(window);
    return true;
}

void DisplayGroupController::scale(const QSizeF& factor)
{
    const QTransform t = QTransform::fromScale(factor.width(), factor.height());

    for (auto&& window : _group.getWindows())
        window->setCoordinates(t.mapRect(window->getCoordinates()));

    _group.setCoordinates(t.mapRect(_group.getCoordinates()));
}

void DisplayGroupController::adjust(const QSizeF& maxGroupSize)
{
    auto targetSize = _group.size().scaled(maxGroupSize, Qt::KeepAspectRatio);
    const qreal scaleFactor = targetSize.width() / _group.width();
    scale(QSizeF(scaleFactor, scaleFactor));
}

void DisplayGroupController::reshape(const QSizeF& newSize)
{
    adjust(newSize);
    _extend(newSize);
}

void DisplayGroupController::denormalize(const QSizeF& targetSize)
{
    if (_group.getCoordinates() != UNIT_RECTF)
        throw std::runtime_error("Target DisplayGroup is not normalized!");

    const qreal aspectRatio = _estimateAspectRatio();
    const QSizeF scaleFactor =
        QSizeF(aspectRatio, 1.0).scaled(targetSize, Qt::KeepAspectRatio);
    scale(scaleFactor);
    // Make sure aspect ratio is 100% correct for all windows - some may be
    // slightly off due to numerical imprecisions in the xml session file.
    adjustWindowsAspectRatioToContent();
}

void DisplayGroupController::adjustWindowsAspectRatioToContent()
{
    for (auto&& window : _group.getWindows())
    {
        QSizeF exactSize = window->getContent().getDimensions();
        exactSize.scale(window->getCoordinates().size(), Qt::KeepAspectRatio);
        window->setWidth(exactSize.width());
        window->setHeight(exactSize.height());
    }
}

QRectF DisplayGroupController::estimateSurface() const
{
    auto area = UNIT_RECTF;
    for (const auto& window : _group.getWindows())
        area = area.united(window->getCoordinates());
    area.setTopLeft(QPointF(0.0, 0.0));

    return area;
}

void DisplayGroupController::updateFocusedWindowsCoordinates()
{
    _updateFocusedCoordinates(_group.getFocusedWindows());
}

void DisplayGroupController::_focus(const WindowSet& windows)
{
    unfocusAll(); // ensure precondition, but should already be empty

    // Update focused coordinates BEFORE focusing windows for proper transition
    _updateFocusedCoordinates(windows);

    for (const auto& window : windows)
    {
        _group.addFocusedWindow(window);
        window->getContent().setCaptureInteraction(true);
    }
}

void DisplayGroupController::_updateFocusedCoordinates(const WindowSet& windows)
{
    LayoutEngine::create(_group)->updateFocusedCoord(windows);
}

void DisplayGroupController::_extend(const QSizeF& newSize)
{
    const auto offset = 0.5 * (newSize - _group.getCoordinates().size());

    _group.setWidth(newSize.width());
    _group.setHeight(newSize.height());

    const auto t = QTransform::fromTranslate(offset.width(), offset.height());
    for (auto&& window : _group.getWindows())
        window->setCoordinates(t.mapRect(window->getCoordinates()));
}

void DisplayGroupController::_showFullscreen(WindowPtr window,
                                             const bool oneToOne)
{
    exitFullscreen();

    window->backupModeAndZoom();

    const auto target = WindowController::Coordinates::FULLSCREEN;
    WindowController controller(*window, _group, target);
    controller.adjustSize(oneToOne ? SIZE_FULLSCREEN_1TO1 : SIZE_FULLSCREEN);

    window->setMode(Window::WindowMode::FULLSCREEN);
    window->getContent().setCaptureInteraction(false);
    _group.setFullscreenWindow(window);
}

qreal DisplayGroupController::_estimateAspectRatio() const
{
    qreal averageAR = 0.0;
    for (const auto& window : _group.getWindows())
    {
        const qreal windowAR = window->width() / window->height();
        averageAR += window->getContent().getAspectRatio() / windowAR;
    }
    averageAR /= _group.getWindows().size();
    return averageAR;
}

void DisplayGroupController::_readjustToNewZoomLevel(Window& window)
{
    WindowController{window, _group}.scale(window.center(), 0.0);
    if (!window.getContent().isZoomed())
        window.getContent().setCaptureInteraction(false);
}
