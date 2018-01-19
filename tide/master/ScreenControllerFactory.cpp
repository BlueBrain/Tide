/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
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

#include "ScreenControllerFactory.h"

#include "MultiScreenController.h"

#include <QMap>
#include <QStringList>

namespace
{
PlanarController::Type _getType(const QString& name)
{
    if (name == "UR9850")
        return PlanarController::Type::TV_UR9850;
    if (name == "UR9851")
        return PlanarController::Type::TV_UR9851;
    return PlanarController::Type::Matrix;
}
}

std::unique_ptr<ScreenController> ScreenControllerFactory::create(
    const QString& ports)
{
    const auto connections = parseInputString(ports);
    if (connections.empty())
        return nullptr;

    std::vector<std::unique_ptr<ScreenController>> controllers;
    for (const auto& kv : connections.toStdMap())
    {
        controllers.emplace_back(new PlanarController(kv.first, kv.second));
    }
    if (controllers.size() == 1)
        return std::move(controllers[0]);

    return std::unique_ptr<ScreenController>(
        new MultiScreenController(std::move(controllers)));
}

QMap<QString, PlanarController::Type> ScreenControllerFactory::parseInputString(
    const QString& ports)
{
    QMap<QString, PlanarController::Type> map;

    const auto connections = ports.split(';');
    if (connections.empty())
        return map;
    for (const auto& connection : connections)
    {
        if (connection.isEmpty())
            continue;

        if (!connection.endsWith("#") && connection.contains("#"))
        {
            const auto serialEntity = connection.split("#");
            const auto& serialPort = serialEntity[0];
            if (serialPort.isEmpty())
                continue;
            const auto type = _getType(serialEntity[1]);
            map.insert(serialPort, type);
        }
        else
        {
            const auto& serialPort = connection;
            const auto type = _getType("");
            map.insert(serialPort, type);
        }
    }
    return map;
}
