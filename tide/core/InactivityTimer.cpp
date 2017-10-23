/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
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

#include "InactivityTimer.h"

#include "log.h"

namespace
{
const int COUNTDOWNTIMER = 15000; // ms
}

InactivityTimer::InactivityTimer()
{
}

InactivityTimer::InactivityTimer(const int timeout)
    : _timeout(timeout * 60000) // from ms to minute
    , _countDownTimer(new QTimer())
    , _inactivityTimer(new QTimer())
{
    _inactivityTimer->setInterval(_timeout);
    _inactivityTimer->setSingleShot(true);
    _inactivityTimer->start();
    _countDownTimer->setSingleShot(true);
    _countDownTimer->setInterval(COUNTDOWNTIMER);

    connect(_inactivityTimer.get(), &QTimer::timeout, [this]() {
        if (!_countDownTimer->isActive())
        {
            _countdownActive = true;
            _countDownTimer->start();
            emit updated(shared_from_this());
        }
    });
    connect(_countDownTimer.get(), &QTimer::timeout, [this]() {
        emit poweroff();
        _countdownActive = false;
        emit updated(shared_from_this());
    });
}

int InactivityTimer::getCountdownTimeout()
{
    return COUNTDOWNTIMER;
}

bool InactivityTimer::isCountdownActive()
{
    return _countdownActive;
}

void InactivityTimer::stop()
{
    _inactivityTimer->stop();
    if (_countdownActive)
    {
        _countdownActive = false;
        _countDownTimer->stop();
        emit updated(shared_from_this());
    }
}

void InactivityTimer::restart()
{
    _inactivityTimer->start();
    if (_countdownActive)
    {
        put_facility_flog(
            LOG_INFO, LOG_POWER,
            "Prevented powering off the screens during countdown");
        _countDownTimer->stop();
        _countdownActive = false;
        emit updated(shared_from_this());
    }
}
