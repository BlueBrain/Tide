/*********************************************************************/
/* Copyright (c) 2017-2018, EPFL/Blue Brain Project                  */
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

#include "MultiScreenController.h"

#include "PlanarController.h"
#include "configuration/Configuration.h"

namespace
{
ScreenState _getResult(const std::vector<ScreenState>& results)
{
    const auto state = results.empty() ? ScreenState::undefined : results[0];
    for (const auto& result : results)
    {
        if (result != state)
            return ScreenState::undefined;
    }
    return state;
}

bool _getResult(const std::vector<bool>& results)
{
    return std::find(results.begin(), results.end(), false) == results.end();
}

template <typename T>
std::function<void(T)> _makeSharedCallback(std::function<void(T)> callback,
                                           const size_t resultsCount)
{
    auto results = std::make_shared<std::vector<T>>();
    return [=](const T result) {
        results->push_back(result);
        if (results->size() == resultsCount && callback)
            callback(_getResult(*results));
    };
}
}

MultiScreenController::MultiScreenController(
    std::vector<std::unique_ptr<ScreenController>>&& controllers)
    : _controllers{std::move(controllers)}
{
    for (auto& controller : _controllers)
        connect(controller.get(), &ScreenController::powerStateChanged,
                [this]() { emit powerStateChanged(getState()); });
}

ScreenState MultiScreenController::getState() const
{
    const auto state = _controllers[0]->getState();
    for (const auto& controller : _controllers)
    {
        if (controller->getState() != state)
            return ScreenState::undefined;
    }
    return state;
}

void MultiScreenController::checkState(ScreenStateCallback callback)
{
    auto sharedCallback = _makeSharedCallback(callback, _controllers.size());
    for (const auto& controller : _controllers)
        controller->checkState(sharedCallback);
}

void MultiScreenController::powerOn(BoolCallback callback)
{
    auto sharedCallback = _makeSharedCallback(callback, _controllers.size());
    for (const auto& controller : _controllers)
        controller->powerOn(sharedCallback);
}

void MultiScreenController::powerOff(BoolCallback callback)
{
    auto sharedCallback = _makeSharedCallback(callback, _controllers.size());
    for (const auto& controller : _controllers)
        controller->powerOff(sharedCallback);
}
