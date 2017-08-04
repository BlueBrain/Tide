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

#define BOOST_TEST_MODULE SwapSynchronizerTests

#include <boost/test/unit_test.hpp>

#include "SwapSynchronizer.h"
#include "SwapSynchronizerHardware.h"
#include "SwapSynchronizerSoftware.h"

#include "MockNetworkBarrier.h"
#include "QGuiAppFixture.h"

BOOST_AUTO_TEST_CASE(testSoftwareSwapSynchronizerFactory)
{
    MockNetworkBarrier barrier;
    auto factory = SwapSynchronizerFactory::get(SwapSync::software);
    BOOST_REQUIRE(factory);
    auto synchronizer = factory->create(barrier, 1);
    BOOST_REQUIRE(synchronizer);
    BOOST_CHECK(dynamic_cast<SwapSynchronizerSoftware*>(synchronizer.get()));
}

BOOST_AUTO_TEST_CASE(testHardwareSwapSynchronizerFactory)
{
    MockNetworkBarrier barrier;
    auto factory = SwapSynchronizerFactory::get(SwapSync::hardware);
    BOOST_REQUIRE(factory);
    auto synchronizer = factory->create(barrier, 1);
    BOOST_REQUIRE(synchronizer);
    BOOST_CHECK(dynamic_cast<SwapSynchronizerHardware*>(synchronizer.get()));
}

struct Hardware : QGuiAppFixture
{
    MockNetworkBarrier barrier;
    std::unique_ptr<SwapSynchronizer> synchronizer =
        SwapSynchronizerFactory::get(SwapSync::hardware)->create(barrier, 1);
    QWindow window;
};

BOOST_FIXTURE_TEST_CASE(testHardwareBarrierWithoutGLContextThrows, Hardware)
{
    BOOST_CHECK_THROW(synchronizer->globalBarrier(window), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE(testExitHardwareBarrierWithoutInitDoesNotThrow,
                        Hardware)
{
    BOOST_CHECK_NO_THROW(synchronizer->exitBarrier(window));
}

struct Software : QGuiAppFixture
{
    MockNetworkBarrier barrier;
    std::unique_ptr<SwapSynchronizer> synchronizer =
        SwapSynchronizerFactory::get(SwapSync::software)->create(barrier, 3);
    QWindow window;
};

BOOST_FIXTURE_TEST_CASE(testSoftwareBarrierWithThreeRenderThreads, Software)
{
    const int nFrames = 10;
    auto renderLoop = [&] {
        for (int i = 0; i < nFrames; ++i)
        {
            BOOST_CHECK_EQUAL(barrier.called, i);
            synchronizer->globalBarrier(window);
            BOOST_CHECK_EQUAL(barrier.called, i + 1);
        }
        BOOST_CHECK_NO_THROW(synchronizer->exitBarrier(window));
    };
    std::thread t1{renderLoop};
    BOOST_CHECK_EQUAL(barrier.called, 0);
    std::thread t2{renderLoop};
    BOOST_CHECK_EQUAL(barrier.called, 0);
    std::thread t3{renderLoop};
    t1.join();
    t2.join();
    t3.join();
    BOOST_CHECK_EQUAL(barrier.called, nFrames);
}
