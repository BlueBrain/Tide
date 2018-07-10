/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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

#include "DoubleTapDetector.h"

#include "MathUtils.h"

DoubleTapDetector::DoubleTapDetector(const qreal doubleTapThresholdPx,
                                     const uint doubleTapTimeoutMs)
    : _doubleTapThresholdPx(doubleTapThresholdPx)
{
    _doubleTapTimer.setInterval(doubleTapTimeoutMs);
    _doubleTapTimer.setSingleShot(true);

    connect(&_doubleTapTimer, &QTimer::timeout, this,
            &DoubleTapDetector::cancelGesture);
}

void DoubleTapDetector::initGesture(const Positions& positions)
{
    if (!_canBeDoubleTap)
    {
        // adding initial points
        if (positions.size() > _touchStartPos.size())
        {
            if (_touchStartPos.empty())
                _startGesture(positions);
            else
                _touchStartPos = positions;
            return;
        }

        // all points released, decide if potential double tap or abort
        if (positions.empty())
        {
            _canBeDoubleTap = _doubleTapTimer.isActive();
            if (!_canBeDoubleTap)
                cancelGesture();
        }
        return;
    }

    // points pressed again, check for double tap
    if (positions.size() == _touchStartPos.size() &&
        !MathUtils::hasMoved(positions, _touchStartPos, _doubleTapThresholdPx))
    {
        emit doubleTap(MathUtils::computeCenter(_touchStartPos),
                       _touchStartPos.size());
        _doubleTapTimer.stop();
    }

    // all points released, reset everything for next detection
    if (positions.empty())
        cancelGesture();
}

void DoubleTapDetector::cancelGesture()
{
    _doubleTapTimer.stop();
    _canBeDoubleTap = false;
    _touchStartPos.clear();
}

void DoubleTapDetector::_startGesture(const Positions& positions)
{
    _touchStartPos = positions;
    _doubleTapTimer.start();
}
