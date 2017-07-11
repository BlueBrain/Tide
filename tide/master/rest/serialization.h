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

#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "types.h"

#include "json.h"

#include <QUuid>

/**
 * Serialize an object as a JSON string.
 *
 * @param object to serialize.
 * @return json string, empty on error.
 */
template <typename Obj>
std::string to_json(const Obj& object)
{
    return json::toString(to_json_object(object));
}

/**
 * Deserialize an object from a JSON string.
 *
 * @param object to serialize.
 * @return json string, empty on error.
 */
template <typename Obj>
bool from_json(Obj& object, const std::string& json)
{
    return from_json_object(object, json::toObject(json));
}

/** @name JSON serialization of objects. */
//@{
QJsonObject to_json_object(ContentWindowPtr window, const DisplayGroup& group);
QJsonObject to_json_object(const DisplayGroup& group);
QJsonObject to_json_object(const LoggingUtility& logger);
QJsonObject to_json_object(const MasterConfiguration& config);
QJsonObject to_json_object(const Options& options);
QJsonObject to_json_object(const QSize& size);
//@}

/** @name JSON deserialization of objects. */
//@{
bool from_json_object(Options& options, const QJsonObject& object);
//@}

/** @name Helpers for QUuid (strip '{}' in JSON form). */
//@{
QString url_encode(const QUuid& uuid);
QUuid url_decode(const QString& uuid);
//@}

#endif
