/*********************************************************************/
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
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

#include "ContentController.h"

#include "config.h"
#include "scene/Window.h"

#if TIDE_ENABLE_PDF_SUPPORT
#include "PDFController.h"
#endif
#if TIDE_ENABLE_MOVIE_SUPPORT
#include "MovieController.h"
#endif
#include "PixelStreamController.h"
#include "ZoomController.h"

std::unique_ptr<ContentController> ContentController::create(Window& window)
{
    switch (window.getContent().getType())
    {
    case ContentType::pixel_stream:
        return std::make_unique<PixelStreamController>(window);
#if TIDE_ENABLE_WEBBROWSER_SUPPORT
    case ContentType::webbrowser:
        return std::make_unique<PixelStreamController>(window);
#endif
#if TIDE_ENABLE_PDF_SUPPORT
    case ContentType::pdf:
        return std::make_unique<PDFController>(window);
#endif
    case ContentType::image:
    case ContentType::svg:
#if TIDE_USE_TIFF
    case ContentType::image_pyramid:
#endif
        return std::make_unique<ZoomController>(window);
#if TIDE_ENABLE_MOVIE_SUPPORT
    case ContentType::movie:
        return std::make_unique<MovieController>(window);
#endif
    case ContentType::invalid:
    default:
        return std::make_unique<ContentController>(window);
    }
}

ContentController::ContentController(Window& window)
    : _window(window)
{
}

void ContentController::prevPage()
{
    _prevPage();
}

void ContentController::nextPage()
{
    _nextPage();
}

void ContentController::touchBegin(const QPointF& position)
{
    _touchBegin(position);
}

void ContentController::touchEnd(const QPointF& position)
{
    _touchEnd(position);
}

void ContentController::addTouchPoint(int id, const QPointF& position)
{
    _addTouchPoint(id, position);
}

void ContentController::updateTouchPoint(int id, const QPointF& position)
{
    _updateTouchPoint(id, position);
}

void ContentController::removeTouchPoint(int id, const QPointF& position)
{
    _removeTouchPoint(id, position);
}

void ContentController::tap(const QPointF& position, uint numPoints)
{
    _tap(position, numPoints);
}

void ContentController::doubleTap(const QPointF& position, uint numPoints)
{
    if (!getWindow().isFocused())
        getContent().setCaptureInteraction(false);
    _doubleTap(position, numPoints);
}

void ContentController::tapAndHold(const QPointF& position, uint numPoints)
{
    if (!getWindow().isFocused())
        getContent().setCaptureInteraction(false);
    _tapAndHold(position, numPoints);
}

void ContentController::pan(const QPointF& position, const QPointF& delta,
                            uint numPoints)
{
    _pan(position, delta, numPoints);
}

void ContentController::pinch(const QPointF& position,
                              const QPointF& pixelDelta)
{
    _pinch(position, pixelDelta);
}

void ContentController::swipeLeft()
{
    _swipeLeft();
}

void ContentController::swipeRight()
{
    _swipeRight();
}

void ContentController::swipeUp()
{
    _swipeUp();
}

void ContentController::swipeDown()
{
    _swipeDown();
}

void ContentController::keyPress(int key, int modifiers, const QString& text)
{
    _keyPress(key, modifiers, text);
}

void ContentController::keyRelease(int key, int modifiers, const QString& text)
{
    _keyRelease(key, modifiers, text);
}

QSizeF ContentController::getWindowSize() const
{
    return _window.getDisplayCoordinates().size();
}

Content& ContentController::getContent()
{
    return _window.getContent();
}

const Content& ContentController::getContent() const
{
    return _window.getContent();
}

const Window& ContentController::getWindow() const
{
    return _window;
}
