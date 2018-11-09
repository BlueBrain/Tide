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

#include "TapAndHoldDetector.h"

#include "MathUtils.h"

TapAndHoldDetector::TapAndHoldDetector(const uint tapAndHoldTimeoutMs,
                                       const qreal moveThresholdPx)
    : _moveThresholdPx(moveThresholdPx)
{
    _tapAndHoldTimer.setInterval(tapAndHoldTimeoutMs);
    _tapAndHoldTimer.setSingleShot(true);
    connect(&_tapAndHoldTimer, &QTimer::timeout, [this]() {
        emit tapAndHold(MathUtils::computeCenter(_touchStartPos),
                        _touchStartPos.size());
    });
}

void TapAndHoldDetector::initGesture(const Positions& positions)
{
    cancelGesture();

    if (!positions.empty())
    {
        _touchStartPos = positions;
        _tapAndHoldTimer.start();
    }
}

void TapAndHoldDetector::updateGesture(const Positions& positions)
{
    if (_tapAndHoldTimer.isActive())
        _cancelGestureIfMoved(positions);
}

void TapAndHoldDetector::cancelGesture()
{
    _tapAndHoldTimer.stop();
}

void TapAndHoldDetector::_cancelGestureIfMoved(const Positions& positions)
{
    if (MathUtils::hasMoved(positions, _touchStartPos, _moveThresholdPx))
        cancelGesture();
}
