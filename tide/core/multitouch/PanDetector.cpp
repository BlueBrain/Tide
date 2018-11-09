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

#include "PanDetector.h"

#include "multitouch/MathUtils.h"

PanDetector::PanDetector(const qreal panThreshold)
    : _panThreshold(panThreshold)
{
}

void PanDetector::initGesture(const Positions& positions)
{
    cancelGesture();

    _startPanPos = MathUtils::computeCenter(positions);
}

void PanDetector::updateGesture(const Positions& positions)
{
    const auto pos = MathUtils::computeCenter(positions);

    if (!_panning && (pos - _startPanPos).manhattanLength() > _panThreshold)
    {
        _startGesture(pos, positions.size());
        _lastPanPos = pos;
    }
    if (_panning)
    {
        emit pan(pos, pos - _lastPanPos, positions.size());
        _lastPanPos = pos;
    }
}

void PanDetector::cancelGesture()
{
    if (!_panning)
        return;

    _panning = false;
    emit panEnded();
}

qreal PanDetector::getPanThreshold() const
{
    return _panThreshold;
}

void PanDetector::setPanThreshold(const qreal arg)
{
    _panThreshold = arg;
}

void PanDetector::_startGesture(const QPointF& pos, const uint numPoints)
{
    if (_panning)
        return;

    _panning = true;
    emit panStarted(pos, numPoints);
}
