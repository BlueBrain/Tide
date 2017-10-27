/*********************************************************************/
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project                  */
/*                          Pawel Podhajski <pawel.podhajski@epfl.ch>*/
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

#include "LoggingUtility.h"

#include "scene/ContentWindow.h"

#include <chrono>

namespace
{
uint _getMilliseconds(std::chrono::system_clock::duration timePoint)
{
    timePoint -= std::chrono::duration_cast<std::chrono::seconds>(timePoint);
    return timePoint.count();
}

QString _getTimestamp()
{
    // ISO extended format: "2016-09-23T10:31:36.776385"
    const auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    std::vector<char> buf(21);
    std::strftime(buf.data(), buf.size(), "%FT%T", std::localtime(&now_c));
    const auto ms = _getMilliseconds(now.time_since_epoch());
    return QString{buf.data()} + '.' + QString::number(ms).left(6);
}

const auto windowAdded = "contentWindowAdded";
const auto windowModeChanged = "mode changed";
const auto windowStateChanged = "state changed";
const auto windowMovedToFront = "contentWindowMovedToFront";
const auto windowRemoved = "contentWindowRemoved";
}

size_t LoggingUtility::getWindowCount() const
{
    return _windowCount;
}

size_t LoggingUtility::getAccumulatedWindowCount() const
{
    return _accumulatedWindowCount;
}

const QString& LoggingUtility::getWindowCountModificationTime() const
{
    return _windowCountModificationTime;
}

int LoggingUtility::getInteractionCount() const
{
    return _interactionCounter;
}

const QString& LoggingUtility::getLastInteractionName() const
{
    return _lastInteractionName;
}

const QString& LoggingUtility::getLastInteractionTime() const
{
    return _lastInteractionTime;
}

ScreenState LoggingUtility::getScreenState() const
{
    return _screenState;
}

QString LoggingUtility::getScreenStateModificationTime() const
{
    return _screenStateModificationTime;
}

void LoggingUtility::logContentWindowAdded(ContentWindowPtr window)
{
    connect(window.get(), &ContentWindow::stateChanged,
            [this]() { _logInteraction(windowStateChanged); });

    connect(window.get(), &ContentWindow::hiddenChanged,
            [this](const bool hidden) {
                hidden ? _decrementWindowCount() : _incrementWindowCount();
            });

    connect(window.get(), &ContentWindow::modeChanged,
            [this]() { _logInteraction(windowModeChanged); });

    if (!window->isHidden())
        _incrementWindowCount();

    ++_accumulatedWindowCount;

    _logInteraction(windowAdded);
}

void LoggingUtility::logContentWindowMovedToFront()
{
    _logInteraction(windowMovedToFront);
}

void LoggingUtility::logContentWindowRemoved()
{
    _decrementWindowCount();
    _logInteraction(windowRemoved);
}

void LoggingUtility::logScreenStateChanged(const ScreenState state)
{
    _screenState = state;
    _screenStateModificationTime = _getTimestamp();
}

void LoggingUtility::_incrementWindowCount()
{
    ++_windowCount;
    _windowCountModificationTime = _getTimestamp();
}

void LoggingUtility::_decrementWindowCount()
{
    if (_windowCount > 0)
        --_windowCount;
    _windowCountModificationTime = _getTimestamp();
}

void LoggingUtility::_logInteraction(const QString& name)
{
    ++_interactionCounter;
    _lastInteractionName = name;
    _lastInteractionTime = _getTimestamp();
}
