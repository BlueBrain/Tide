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

#ifndef JSON_SERIALIZATION_H
#define JSON_SERIALIZATION_H

#include "types.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QUuid>

struct Screen;

namespace json
{
/** @name Helpers for QUuid (strip '{}' in JSON form). */
//@{
QString url_encode(const QUuid& uuid);
QUuid url_decode(const QString& uuid);
//@}

/** @name Serialize custom enum types as JSON value. */
//@{
QJsonValue serialize(SwapSync result);
QJsonValue serialize(deflect::View result);
//@}

/** @name Get a value as a specific type. */
//@{
void deserialize(const QJsonValue& value, bool& result);
void deserialize(const QJsonValue& value, int& result);
void deserialize(const QJsonValue& value, uint& result);
void deserialize(const QJsonValue& value, ushort& result);
void deserialize(const QJsonValue& value, double& result);
void deserialize(const QJsonValue& value, QString& result);
void deserialize(const QJsonValue& value, SwapSync& result);
void deserialize(const QJsonValue& value, deflect::View& result);
//@}

/** @name JSON serialization of objects. */
//@{
QJsonArray serialize(const QSize& size);
QJsonArray serialize(const QSizeF& size);
QJsonObject serializeAsObject(const QSize& size);
QJsonObject serialize(const Background& background);
QJsonObject serialize(const Options& options);
QJsonObject serialize(const Configuration& config);
QJsonObject serialize(const Process& process);
QJsonObject serialize(const Screen& screen);
QJsonObject serialize(const SurfaceConfig& surface);
//@}

/** @name JSON deserialization of objects. */
//@{
bool deserialize(const QJsonValue& value, QSize& size);
bool deserialize(const QJsonValue& value, QSizeF& size);
bool deserialize(const QJsonArray& array, QSize& size);
bool deserialize(const QJsonArray& array, QSizeF& size);
bool deserialize(const QJsonObject& object, QSize& size);
bool deserialize(const QJsonObject& object, QSizeF& size);
bool deserialize(const QJsonObject& object, Background& background);
bool deserialize(const QJsonObject& object, Options& options);
bool deserialize(const QJsonObject& object, Configuration& config);
bool deserialize(const QJsonObject& object, Process& process);
bool deserialize(const QJsonObject& object, Screen& screen);
bool deserialize(const QJsonObject& object, SurfaceConfig& surface);
//@}
}

#endif
