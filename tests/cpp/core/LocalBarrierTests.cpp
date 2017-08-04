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

#define BOOST_TEST_MODULE LocalBarrierTests

#include <boost/test/unit_test.hpp>

#include "network/LocalBarrier.h"

#include <atomic>
#include <thread>

struct Fixture
{
    std::atomic<int> caller = {0};
    std::atomic<int> called = {0};
    void action(const int value)
    {
        caller = value;
        ++called;
    }
};

BOOST_FIXTURE_TEST_CASE(testSingleThread, Fixture)
{
    LocalBarrier barrier{1};
    barrier.waitForAllThreadsThen([&] { action(1); });
    BOOST_CHECK_EQUAL(called, 1);
    BOOST_CHECK_EQUAL(caller, 1);
}

BOOST_FIXTURE_TEST_CASE(testNoThreadIsSameAsSingleThread, Fixture)
{
    LocalBarrier barrier{0};
    barrier.waitForAllThreadsThen([&] { action(1); });
    BOOST_CHECK_EQUAL(called, 1);
    BOOST_CHECK_EQUAL(caller, 1);
}

BOOST_FIXTURE_TEST_CASE(testTwoThreads, Fixture)
{
    LocalBarrier barrier{2};
    std::thread t1{[&] { barrier.waitForAllThreadsThen([&] { action(1); }); }};
    BOOST_CHECK_EQUAL(caller, 0);
    std::thread t2{[&] { barrier.waitForAllThreadsThen([&] { action(2); }); }};
    t1.join();
    t2.join();
    BOOST_CHECK_EQUAL(called, 1);
    // The order in which the std::threads join the barrier is not guaranteed
    BOOST_CHECK(caller == 1 || caller == 2);
}

BOOST_FIXTURE_TEST_CASE(testThreeThreads, Fixture)
{
    LocalBarrier barrier{3};
    std::thread t3{[&] { barrier.waitForAllThreadsThen([&] { action(3); }); }};
    BOOST_CHECK_EQUAL(caller, 0);
    std::thread t1{[&] { barrier.waitForAllThreadsThen([&] { action(1); }); }};
    BOOST_CHECK_EQUAL(caller, 0);
    std::thread t2{[&] { barrier.waitForAllThreadsThen([&] { action(2); }); }};
    BOOST_CHECK_EQUAL(caller, 0);
    t1.join();
    t2.join();
    t3.join();
    BOOST_CHECK_EQUAL(called, 1);
    // The order in which the std::threads join the barrier is not guaranteed
    BOOST_CHECK(caller >= 1 && caller <= 3);
}
