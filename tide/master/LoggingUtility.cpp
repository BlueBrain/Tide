/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
/*                     Pawel Podhajski <pawel.podhajski@epfl.ch>     */
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

size_t LoggingUtility::getAccumulatedWindowCount() const
{
    return _windowCounterTotal;
}

const QString& LoggingUtility::getCounterModificationTime() const
{
    return _counterModificationTime;
}

int LoggingUtility::getInteractionCount() const
{
    return _interactionCounter;
}

const QString& LoggingUtility::getLastInteraction() const
{
    return _lastInteraction;
}

const QString& LoggingUtility::getLastInteractionTime() const
{
    return _lastInteractionTime;
}

size_t LoggingUtility::getWindowCount() const
{
    return _windowCounter;
}

void LoggingUtility::contentWindowAdded(ContentWindowPtr contentWindow)
{
    connect(contentWindow.get(), &ContentWindow::stateChanged,
            [this]() { _log("state changed"); });

    connect(contentWindow.get(), &ContentWindow::hiddenChanged,
            [this](bool hidden) {
                hidden ? _decrementWindowCount() : _incrementWindowCount();
            });

    connect(contentWindow.get(), &ContentWindow::modeChanged,
            [this]() { _log("mode changed"); });

    _incrementWindowCount();
    ++_windowCounterTotal;
    _counterModificationTime = _getTimeStamp();
    _log(__func__);
}

void LoggingUtility::contentWindowMovedToFront()
{
    _log(__func__);
}

void LoggingUtility::contentWindowRemoved()
{
    _decrementWindowCount();
    _counterModificationTime = _getTimeStamp();
    _log(__func__);
}

void LoggingUtility::_decrementWindowCount()
{
    if (_windowCounter > 0)
        --_windowCounter;
}

void LoggingUtility::_incrementWindowCount()
{
    ++_windowCounter;
}

void LoggingUtility::powerStateChanged(const ScreenState state)
{
    _state = state;
    _lastPowerStateChanged = _getTimeStamp();
}

QString LoggingUtility::getLastScreenStateChanged() const
{
    return _lastPowerStateChanged;
}

ScreenState LoggingUtility::getScreenState() const
{
    return _state;
}

uint _getMilliseconds(std::chrono::system_clock::duration timePoint)
{
    timePoint -= std::chrono::duration_cast<std::chrono::seconds>(timePoint);
    return timePoint.count();
}

QString LoggingUtility::_getTimeStamp() const
{
    // ISO extended format: "2016-09-23T10:31:36.776385"
    const auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    std::vector<char> buf(21);
    std::strftime(buf.data(), buf.size(), "%FT%T", std::localtime(&now_c));
    const auto ms = _getMilliseconds(now.time_since_epoch());
    return QString{buf.data()} + '.' + QString::number(ms).left(6);
}

void LoggingUtility::_log(const QString& s)
{
    _lastInteraction = s;
    _lastInteractionTime = _getTimeStamp();
    ++_interactionCounter;
}
