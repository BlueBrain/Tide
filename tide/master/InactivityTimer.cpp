/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
/*                     Pawel Podhajski <pawel.podhajski@epfl.ch>     */
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

#include "InactivityTimer.h"

#include "CountdownStatus.h"
#include "log.h"

namespace
{
const auto COUNTDOWNTIMER_MS = 15000;
const auto MS_PER_MIN = 60000;
}

InactivityTimer::InactivityTimer(const uint timeoutInMinutes)
{
    _inactivityTimer.setInterval(timeoutInMinutes * MS_PER_MIN);
    _inactivityTimer.setSingleShot(true);
    _inactivityTimer.start();

    _countdownTimer.setInterval(COUNTDOWNTIMER_MS);
    _countdownTimer.setSingleShot(true);

    connect(&_inactivityTimer, &QTimer::timeout, [this]() {
        if (!_countdownTimer.isActive())
        {
            _countdownTimer.start();
            _sendCountdownStatus();
        }
    });
    connect(&_countdownTimer, &QTimer::timeout, [this]() {
        emit poweroff();
        _sendCountdownStatus();
    });
}

void InactivityTimer::stop()
{
    _inactivityTimer.stop();
    if (_countdownTimer.isActive())
    {
        _countdownTimer.stop();
        _sendCountdownStatus();
    }
}

void InactivityTimer::restart()
{
    _inactivityTimer.start();
    if (_countdownTimer.isActive())
    {
        print_log(LOG_INFO, LOG_POWER,
                  "Prevented powering off the screens during countdown");
        _countdownTimer.stop();
        _sendCountdownStatus();
    }
}

void InactivityTimer::_sendCountdownStatus()
{
    emit countdownUpdated(
        boost::make_shared<CountdownStatus>(_countdownTimer.isActive(),
                                            (uint)_countdownTimer.interval()));
}
