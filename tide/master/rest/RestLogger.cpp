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

#include "RestLogger.h"

#include "json.h"
#include "jsonschema.h"

#include <QJsonObject>

namespace
{
QString stateToString(ScreenState state)
{
    switch (state)
    {
    case ScreenState::ON:
        return "ON";
    case ScreenState::OFF:
        return "OFF";
    case ScreenState::UNDEF:
        return "UNDEF";
    default:
        return "UNDEF";
    }
}
}

QJsonObject _makeJsonObject(const LoggingUtility& logger)
{
    const QJsonObject event{{"last_event", logger.getLastInteraction()},
                            {"last_event_date",
                             logger.getLastInteractionTime()},
                            {"count", logger.getInteractionCount()}};
    const QJsonObject window{{"count", int(logger.getWindowCount())},
                             {"date_set", logger.getCounterModificationTime()},
                             {"accumulated_count",
                              int(logger.getAccumulatedWindowCount())}};
    const QJsonObject screens{{"state", stateToString(logger.getScreenState())},
                              {"last_change",
                               logger.getLastScreenStateChanged()}};
    return QJsonObject{{"event", event},
                       {"window", window},
                       {"screens", screens}};
}

RestLogger::RestLogger(const LoggingUtility& logger)
    : _logger(logger)
{
}

std::string RestLogger::getTypeName() const
{
    return "tide/stats";
}

std::string RestLogger::getSchema() const
{
    return jsonschema::create("Stats", _makeJsonObject(_logger),
                              "Usage statistics of the Tide application");
}

std::string RestLogger::_toJSON() const
{
    return json::toString(_makeJsonObject(_logger));
}
