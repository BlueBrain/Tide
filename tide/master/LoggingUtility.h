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

#ifndef LOGGINGUTILITY_H
#define LOGGINGUTILITY_H

#include <QObject>

#include "types.h"

/**
 * Gathers information/statistics on application usage.
 */
class LoggingUtility : public QObject
{
    Q_OBJECT

public:
    /** @name Number of open windows. */
    //@{
    /** @return the number of currently open windows. */
    size_t getWindowCount() const;

    /** @return the number of open windows from the start. */
    size_t getAccumulatedWindowCount() const;

    /** @return the timestamp of the last opening/closing of a window. */
    const QString& getWindowCountModificationTime() const;
    //@}

    /** @name Interactions with windows. */
    //@{
    /** @return the value of interaction counter. */
    int getInteractionCount() const;

    /** @return the name of last interaction */
    const QString& getLastInteractionName() const;

    /** @return the timestamp of last interaction */
    const QString& getLastInteractionTime() const;
    //@}

    /** @name State of the screen (on/off). */
    //@{
    /** @return state of displays. */
    ScreenState getScreenState() const;

    /** @return the timestamp of last screen state change. */
    QString getScreenStateModificationTime() const;
    //@}

public slots:
    /** Log the event, update the counters and update the timestamp of last
     * interaction */
    void logContentWindowAdded(ContentWindowPtr contentWindow);

    /** Log the event, update the counters and update the timestamp of last
     * interaction */
    void logContentWindowMovedToFront();

    /** Log the event, update the counters and update the timestamp of last
     * interaction */
    void logContentWindowRemoved();

    /** Log the event and update the timestamp of last power action */
    void logScreenStateChanged(ScreenState state);

private:
    size_t _windowCount = 0;
    size_t _accumulatedWindowCount = 0;
    QString _windowCountModificationTime;

    size_t _interactionCounter = 0;
    QString _lastInteractionName;
    QString _lastInteractionTime;

    ScreenState _screenState = ScreenState::UNDEF;
    QString _screenStateModificationTime;

    void _incrementWindowCount();
    void _decrementWindowCount();
    void _logInteraction(const QString& name);
};

#endif
