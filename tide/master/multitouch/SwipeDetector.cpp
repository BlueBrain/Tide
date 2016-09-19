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

#include "SwipeDetector.h"

#include "multitouch/MathUtils.h"

#include <cmath>

SwipeDetector::SwipeDetector( const qreal swipeMaxFingersIntervalPx,
                              const qreal swipeThresholdPx )
    : _swipeMaxFingersIntervalPx( swipeMaxFingersIntervalPx )
    , _swipeThresholdPx( swipeThresholdPx )
{}

void SwipeDetector::initGesture( const QPointF& p0, const QPointF& p1 )
{
    _canBeSwipe = _checkFingersDistanceForSwipe( p0, p1 );
    _swipeStartPos = MathUtils::getCenter( p0, p1 );
}

void SwipeDetector::updateGesture( const QPointF& p0, const QPointF& p1 )
{
    if( !_canBeSwipe )
        return;

    if( !_checkFingersDistanceForSwipe( p0, p1 ))
    {
        cancelGesture();
        return;
    }

    const auto twoFingersPos = MathUtils::getCenter( p0, p1 );
    const auto twoFingersStartPos = _swipeStartPos;
    const auto dist = MathUtils::getDist( twoFingersStartPos, twoFingersPos );
    if( dist > _swipeThresholdPx )
    {
        const qreal dx = twoFingersPos.x() - twoFingersStartPos.x();
        const qreal dy = twoFingersPos.y() - twoFingersStartPos.y();

        if( std::abs( dx ) > std::abs( dy ))
        {
            // Horizontal swipe
            if( dx > 0.0 )
                emit swipeRight();
            else
                emit swipeLeft();
        }
        else
        {
            // Vertical swipe
            if( dy > 0.0 )
                emit swipeDown();
            else
                emit swipeUp();
        }
        cancelGesture(); // Only allow one swipe
    }
}

void SwipeDetector::cancelGesture()
{
    _canBeSwipe = false;
}

bool SwipeDetector::_checkFingersDistanceForSwipe( const QPointF& p0,
                                                   const QPointF& p1 ) const
{
    const qreal fingersInterval = MathUtils::getDist( p0, p1 );
    return fingersInterval < _swipeMaxFingersIntervalPx;
}
