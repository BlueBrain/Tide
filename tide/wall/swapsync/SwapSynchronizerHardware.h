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

#ifndef SWAPSYNCHRONIZERHARDWARE_H
#define SWAPSYNCHRONIZERHARDWARE_H

#include "swapsync/SwapSynchronizer.h"

#include "HardwareSwapGroup.h"
#include "network/SharedNetworkBarrier.h"

// clang-format off
/**
 * Hardware swap synchonizer using GL extensions (for NVidia Quadro GSync card).
 *
 * Given the following setup:
 *
 * Node #1: process A {Left window, Right window}
 * Node #2: process B {Left window, Right window}
 *
 * Where each window has its own (non-sharing) GLContext and render thread.
 * The startup procedure is:
 *                                      — global barrier —
 * A-L joinSwapGroup(1) | A-R joinSwapGroup(1) || B-L joinSwapGroup(1) | B-R joinSwapGroup(1)
 *            — A local barrier —              ||             — B local barrier —
 *           A joinSwapBarrier(1,1)            ||            B joinSwapBarrier(1,1)
 *                                      — global barrier —
 *
 * And the shutdown procedure is:
 *                                      — global barrier —
 *             A leaveSwapBarrier()            ||             B leaveSwapBarrier()
 * A-L leaveSwapGroup() | A-R leaveSwapGroup() || B-L leaveSwapGroup() | B-R leaveSwapGroup()
 *
 * The {join|leave}SwapBarrier must be executed by a single GL context per
 * machine.
 */ // clang-format on
class SwapSynchronizerHardware : public SwapSynchronizer
{
public:
    SwapSynchronizerHardware(NetworkBarrier& barrier, uint windowCount);

    void globalBarrier(const QWindow& window) final;
    void exitBarrier(const QWindow& window) final;

private:
    NetworkBarrier& _networkBarrier;
    SharedNetworkBarrier _globalBarrier;
    LocalBarrier _localBarrier;
    uint _windowCount = 0;
    HardwareSwapGroup _hardwareSwapGroup;
    bool _initialized = false;
};

#endif
