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

#ifndef INACTIVITYTIMER_H
#define INACTIVITYTIMER_H

#include "serialization/includes.h"
#include "types.h"

#include <boost/enable_shared_from_this.hpp>

#include <QObject>
#include <QTimer>

/**
 * Inform user about inactivity timeout.
 */
class InactivityTimer : public QObject,
                        public boost::enable_shared_from_this<InactivityTimer>
{
    Q_OBJECT
    Q_DISABLE_COPY(InactivityTimer)

public:
    /**
     * Construct a timer which can be used to turn off the displays.
     * @param timeout value of the timer in minutes.
     */
    InactivityTimer(int timeout);

    /** Default constructor, creates a read-only timer used on Wall processes */
    InactivityTimer();

    /** Get the duration of countdown needed for transition in qml */
    Q_INVOKABLE int getCountdownTimeout();

    /** Check if countdown timer is active */
    Q_INVOKABLE bool isCountdownActive();

    /** Stop the timer */
    void stop();

    /** Restart the inactivity timer and interrupt the countdown if active */
    void restart();

signals:
    /** Emitted when the countdown timer times-out */
    void poweroff();

    /** Emitted when the state of timer is modified */
    void updated(InactivityTimerPtr);

private:
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        // clang-format off
        ar & _countdownActive;
        // clang-format on
    }

    bool _countdownActive = false;
    int _timeout;

    std::unique_ptr<QTimer> _countDownTimer;
    std::unique_ptr<QTimer> _inactivityTimer;
};
#endif
