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

#include "RestConfiguration.h"

#include "json.h"
#include "scene/ContentFactory.h"

#include <tide/master/version.h>

#include <QDateTime>
#include <QHostInfo>

namespace
{
const QString _startTime = QDateTime::currentDateTime().toString();

QJsonObject _toJsonObject(const QSize& size)
{
    return QJsonObject{{"width", size.width()}, {"height", size.height()}};
}
}

RestConfiguration::RestConfiguration(const MasterConfiguration& config)
    : _config(config)
{
}

std::string RestConfiguration::getTypeName() const
{
    return "tide/config";
}

std::string RestConfiguration::_toJSON() const
{
    QJsonObject config{
        {"hostname", QHostInfo::localHostName()},
        {"version", QString::fromStdString(tide::Version::getString())},
        {"revision", QString::number(tide::Version::getRevision(), 16)},
        {"startTime", _startTime},
        {"wallSize", _toJsonObject(_config.getTotalSize())},
        {"backgroundColor", _config.getBackgroundColor().name()},
        {"contentDir", _config.getContentDir()},
        {"sessionDir", _config.getSessionsDir()},
        {"filters", QJsonArray::fromStringList(
                        ContentFactory::getSupportedFilesFilter())}};
    return json::toString(QJsonObject{{"config", config}});
}
