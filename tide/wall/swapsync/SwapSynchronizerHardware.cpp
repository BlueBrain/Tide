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

#include "SwapSynchronizerHardware.h"

#include "network/WallToWallChannel.h"

namespace
{
const int groupId = 1;
const int barrierId = 1;
}

SwapSynchronizerHardware::SwapSynchronizerHardware(NetworkBarrier& barrier,
                                                   const uint windowCount)
    : _networkBarrier{barrier}
    , _globalBarrier{barrier, windowCount}
    , _localBarrier{windowCount}
    , _windowCount{windowCount}
    , _hardwareSwapGroup{groupId}
{
}

void SwapSynchronizerHardware::globalBarrier(const QWindow& window)
{
    /* All render threads call this function concurrently */

    if (_initialized)
        return;

    _globalBarrier.waitForAll();

    _hardwareSwapGroup.add(window);

    _localBarrier.waitForAllThreadsThen([this]() {
        _hardwareSwapGroup.join(barrierId);
        _initialized = true;
    });

    _globalBarrier.waitForAll();
}

void SwapSynchronizerHardware::exitBarrier(const QWindow& window)
{
    if (!_initialized)
        return;

    /* Render threads call this function sequentially */

    if (_hardwareSwapGroup.size() == _windowCount)
    {
        _networkBarrier.globalBarrier();
        _hardwareSwapGroup.leaveBarrier();
    }
    _hardwareSwapGroup.remove(window);

    if (_hardwareSwapGroup.size() == 0)
        _initialized = false;
}
