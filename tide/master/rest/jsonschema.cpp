/*********************************************************************/
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project                  */
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

#include "jsonschema.h"

#include "json.h"

#include <QStringList>

// forward declaration
QJsonObject _getPropertySchema(const QString& name, const QJsonValue& value);

QString _toString(const QJsonValue::Type type)
{
    switch (type)
    {
    case QJsonValue::Null:
        return "null";
    case QJsonValue::Bool:
        return "boolean";
    case QJsonValue::Double:
        return "number";
    case QJsonValue::String:
        return "string";
    case QJsonValue::Array:
        return "array";
    case QJsonValue::Object:
        return "object";
    case QJsonValue::Undefined:
    default:
        return "";
    }
}

QJsonObject _getValueSchema(const QJsonValue::Type type)
{
    return QJsonObject{{"type", _toString(type)}};
}

QJsonObject _getArraySchema(const QString& name, const QJsonArray& array)
{
    if (array.isEmpty())
        return {};

    return QJsonObject{{"type", "array"},
                       {"title", name},
                       {"items",
                        _getPropertySchema(name + "_items", array.first())}};
}

QJsonObject _getObjectSchema(const QString& title, const QJsonObject& object)
{
    QJsonObject properties;
    for (auto it = object.begin(); it != object.end(); ++it)
        properties[it.key()] = _getPropertySchema(it.key(), it.value());

    return QJsonObject{{"$schema", "http://json-schema.org/schema#"},
                       {"title", title},
                       {"type", "object"},
                       {"additionalProperties", false},
                       {"properties", properties}};
}

QJsonObject _getPropertySchema(const QString& name, const QJsonValue& value)
{
    switch (value.type())
    {
    case QJsonValue::Bool:
    case QJsonValue::Double:
    case QJsonValue::String:
        return _getValueSchema(value.type());
    case QJsonValue::Array:
        return _getArraySchema(name, value.toArray());
    case QJsonValue::Object:
        return _getObjectSchema(name, value.toObject());
    case QJsonValue::Null:
    case QJsonValue::Undefined:
    default:
        return {};
    }
}

namespace jsonschema
{
std::string create(const QString& title, const QJsonObject& object,
                   const QString& description)
{
    auto schema = _getObjectSchema(title, object);
    schema["description"] = description;
    return json::toString(schema);
}

std::string create(const QString& title, const QJsonArray& array,
                   const QString& description, const bool fixedSize)
{
    auto schema = _getArraySchema(title, array);
    schema["description"] = description;
    if (fixedSize)
    {
        schema["minItems"] = array.size();
        schema["maxItems"] = array.size();
    }
    return json::toString(schema);
}
}
