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

#include "PinchDetector.h"

#include "multitouch/MathUtils.h"

#include <cmath>

PinchDetector::PinchDetector(const qreal pinchThresholdPx)
    : _pinchThresholdPx(pinchThresholdPx)
{
}

void PinchDetector::initGesture(const QPointF& pos0, const QPointF& pos1)
{
    const auto twoFingersStartRect = MathUtils::getBoundingRect(pos0, pos1);
    const auto w = twoFingersStartRect.width();
    const auto h = twoFingersStartRect.height();
    _initialPinchDist = std::sqrt(w * w + h * h);
}

void PinchDetector::updateGesture(const QPointF& pos0, const QPointF& pos1)
{
    if (!_pinching)
    {
        const auto pinchDist = MathUtils::getDist(pos0, pos1);
        const auto pinchDelta = std::abs(pinchDist - _initialPinchDist);
        if (pinchDelta > _pinchThresholdPx)
            _startGesture(pos0, pos1);
        else
            return;
    }

    const auto pinchRect = MathUtils::getBoundingRect(pos0, pos1);
    const auto pinchDelta = pinchRect.size() - _lastPinchRect.size();
    _lastPinchRect = pinchRect;
    emit pinch(pinchRect.center(),
               QPointF{pinchDelta.width(), pinchDelta.height()});
}

void PinchDetector::cancelGesture()
{
    if (!_pinching)
        return;

    _pinching = false;
    emit pinchEnded();
}

void PinchDetector::_startGesture(const QPointF& pos0, const QPointF& pos1)
{
    if (_pinching)
        return;

    _pinching = true;
    _lastPinchRect = MathUtils::getBoundingRect(pos0, pos1);
    emit pinchStarted();
}
