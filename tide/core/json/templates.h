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

#ifndef JSON_TEMPLATES_H
#define JSON_TEMPLATES_H

#include "json/json.h"

/**
 * JSON templated helper functions.
 *
 * Must be inluded after all (de)serialize() definitions.
 */
namespace json
{
/** Serialize a collection of objects to a JSON array. */
template <typename IterableT>
QJsonArray serialize(const IterableT& collection)
{
    QJsonArray array;
    for (const auto& item : collection)
        array.append(serialize(item));
    return array;
}

/** @return json text representation of a serializable object. */
template <typename SerializableT>
std::string dump(const SerializableT& object)
{
    return dump(serialize(object));
}

/** @return binary json representation of a serializable object. */
template <typename SerializableT>
QByteArray pack(const SerializableT& object)
{
    return pack(serialize(object));
}

// forward-declare
template <typename DeserializableT>
DeserializableT create(const QJsonObject& object);

/** Deserialize a vector of objects from a JSON array. */
template <typename T>
void deserialize(const QJsonValue& value, std::vector<T>& result)
{
    if (!value.isUndefined())
    {
        std::vector<T> tmp;
        for (const auto& surface : value.toArray())
            tmp.emplace_back(create<T>(surface.toObject()));
        result = std::move(tmp);
    }
}

/** @return deserializable object parsed from a JSON object. */
template <typename DeserializableT>
DeserializableT create(const QJsonObject& object)
{
    DeserializableT tmp;
    deserialize(object, tmp);
    return tmp;
}

/** Deserialize an object from a JSON text representation. */
template <typename DeserializableT>
bool deserialize(const std::string& json, DeserializableT& object)
{
    return deserialize(parse(json), object);
}

/** Deserialize an object from a binary JSON representation. */
template <typename DeserializableT>
DeserializableT unpack(const QByteArray& binaryData)
{
    return create<DeserializableT>(unpack(binaryData));
}
}

#endif
