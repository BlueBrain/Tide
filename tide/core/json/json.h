/*********************************************************************/
/* Copyright (c) 2017-2018, EPFL/Blue Brain Project                  */
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

#ifndef JSON_H
#define JSON_H

#include <QJsonArray>
#include <QJsonObject>

/**
 * Json helper functions.
 */
namespace json
{
/**
 * Parse a json document.
 *
 * @param data json document to parse.
 * @return json object, empty on error.
 */
QJsonObject parse(const std::string& data);

/**
 * Parse a file to an object.
 *
 * @param filename of a json file to parse.
 * @return json object.
 * @throw std::runtime_error on failure.
 */
QJsonObject read(const QString& filename);

/**
 * Write an object to a file.
 *
 * @param object json object to write.
 * @param filename to write.
 * @throw std::runtime_error on failure.
 */
void write(const QJsonObject& object, const QString& filename);

/**
 * Serialize a json array.
 *
 * @param array json array to serialize.
 * @return json string.
 */
std::string dump(const QJsonArray& array);

/**
 * Serialize a json object.
 *
 * @param object json object to serialize.
 * @return json string.
 */
std::string dump(const QJsonObject& object);

/**
 * Pack a json object to Qt's binary format.
 *
 * @param object json object to pack.
 * @return binaray data.
 */
QByteArray pack(const QJsonObject& object);

/**
 * Unpack a json object in Qt's binary format.
 *
 * @param binaryData json data to parse.
 * @return json object, empty on error.
 */
QJsonObject unpack(const QByteArray& binaryData);
}

#endif
