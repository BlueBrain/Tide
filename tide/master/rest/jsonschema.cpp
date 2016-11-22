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

#include "jsonschema.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

// forward declaration
QJsonObject _getPropertySchema( const QString& name, const QJsonValue& value );

QString _toString( const QJsonValue::Type type )
{
    switch( type )
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

QJsonObject _getValueSchema( const QJsonValue::Type type )
{
    QJsonObject valueSchema;
    valueSchema["type"] = _toString( type );
    return valueSchema;
}

QJsonObject _getArraySchema( const QString& name, const QJsonArray& array )
{
    if( array.isEmpty( ))
        return {};

    QJsonObject arraySchema;
    arraySchema["type"] = "array";
    arraySchema["title"] = name;
    arraySchema["items"] = _getPropertySchema( name+"_items", array.first( ));
    return arraySchema;
}

QJsonObject _getObjectSchema( const QString& title, const QJsonObject& object )
{
    QJsonObject schema;
    schema["$schema"] = "http://json-schema.org/schema#";
    schema["title"] = title;
    schema["type"] = "object";
    schema["additionalProperties"] = false;
    QJsonObject properties;
    for( auto it = object.begin(); it != object.end(); ++it )
        properties[it.key()] = _getPropertySchema( it.key(), it.value( ));
    schema["properties"] = properties;
    return schema;
}

QJsonObject _getPropertySchema( const QString& name, const QJsonValue& value )
{
    switch( value.type( ))
    {
    case QJsonValue::Bool:
    case QJsonValue::Double:
    case QJsonValue::String:
        return _getValueSchema( value.type( ));
    case QJsonValue::Array:
        return _getArraySchema( name, value.toArray( ));
    case QJsonValue::Object:
        return _getObjectSchema( name, value.toObject( ));
    case QJsonValue::Null:
    case QJsonValue::Undefined:
    default:
        return {};
    }
}

std::string _toString( const QJsonObject& schema )
{
    const QJsonDocument doc{ schema };
    return doc.toJson( QJsonDocument::JsonFormat::Compact ).toStdString();
}

namespace jsonschema
{

std::string create( const QString& title, const QJsonObject& object,
                    const QString& description )
{
    auto schema = _getObjectSchema( title, object );
    schema["description"] = description;
    return _toString( schema );
}

std::string create( const QString& title, const QJsonArray& array,
                    const QString& description, const bool fixedSize )
{
    auto schema = _getArraySchema( title, array );
    schema["description"] = description;
    if( fixedSize )
    {
        schema["minItems"] = array.size();
        schema["maxItems"] = array.size();
    }
    return _toString( schema );
}

}
