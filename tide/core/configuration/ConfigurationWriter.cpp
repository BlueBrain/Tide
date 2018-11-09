/*********************************************************************/
/* Copyright (c) 2018, EPFL/Blue Brain Project                       */
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

#include "ConfigurationWriter.h"

#include "Configuration.h"
#include "json/json.h"
#include "json/serialization.h"
#include "json/templates.h"

namespace
{
// Forward-declare
QJsonObject removeDefaultValues(const QJsonObject& object,
                                const QJsonObject& defaultObject);

QJsonArray removeDefaultValues(const QJsonArray& array,
                               const QJsonArray& defaultArray)
{
    // Two cases: array of objects or array of values
    if (defaultArray[0].isObject())
    {
        // assume defaultArray contains a single reference object and filter
        // individual objects
        const auto refObject = defaultArray[0].toObject();
        QJsonArray min;
        for (auto i = 0; i < array.size(); ++i)
            min.append(QJsonValue(
                removeDefaultValues(array[i].toObject(), refObject)));
        return min;
    }
    else if (!defaultArray[0].isArray())
    {
        // return full array of values if there is a difference, else nothing
        for (auto i = 0; i < array.size(); ++i)
        {
            if (array[i] != defaultArray[i])
                return array;
        }
    }
    return QJsonArray();
}

QJsonObject removeDefaultValues(const QJsonObject& object,
                                const QJsonObject& defaultObject)
{
    QJsonObject min;
    for (auto it = defaultObject.begin(); it != defaultObject.end(); ++it)
    {
        if (it.value().isObject())
        {
            auto tmp = removeDefaultValues(object[it.key()].toObject(),
                                           it.value().toObject());
            if (!tmp.isEmpty())
                min[it.key()] = tmp;
        }
        else if (it.value().isArray())
        {
            auto tmp = removeDefaultValues(object[it.key()].toArray(),
                                           it.value().toArray());
            if (!tmp.isEmpty())
                min[it.key()] = tmp;
        }
        else if (object[it.key()] != it.value())
            min[it.key()] = object[it.key()];
    }
    return min;
}
}

ConfigurationWriter::ConfigurationWriter(const Configuration& config)
    : _config{config}
{
}

void ConfigurationWriter::write(const QString& filename,
                                const Format format) const
{
    auto jsonObject = json::serialize(_config);
    if (format == Format::minimal)
    {
        const auto defaults = json::serialize(Configuration::defaults());
        jsonObject = removeDefaultValues(jsonObject, defaults);
    }
    json::write(jsonObject, filename);
}
