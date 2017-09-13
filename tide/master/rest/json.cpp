/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
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

#include "json.h"

#include "log.h"

#include <QJsonDocument>

namespace json
{
QJsonArray toArray(const std::string& data)
{
    const auto input = QByteArray::fromRawData(data.c_str(), data.size());
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(input, &error);

    if (doc.isNull() || !doc.isArray())
    {
        put_flog(LOG_INFO, "Error parsing JSON string '%s' '%s'", data.c_str(),
                 error.errorString().toLocal8Bit().constData());
        return QJsonArray{};
    }
    return doc.array();
}

QJsonObject toObject(const std::string& data)
{
    const auto input = QByteArray::fromRawData(data.c_str(), data.size());
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(input, &error);

    if (doc.isNull() || !doc.isObject())
    {
        put_flog(LOG_INFO, "Error parsing JSON string '%s' '%s'", data.c_str(),
                 error.errorString().toLocal8Bit().constData());
        return QJsonObject{};
    }
    return doc.object();
}

std::string toString(const QJsonArray& array)
{
    return QJsonDocument{array}.toJson().toStdString();
}

std::string toString(const QJsonObject& object)
{
    return QJsonDocument{object}.toJson().toStdString();
}
}
