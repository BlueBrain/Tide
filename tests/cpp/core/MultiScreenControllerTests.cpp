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

#define BOOST_TEST_MODULE MutliScreenControllerTest

#include <boost/test/unit_test.hpp>

#include "MockScreenController.h"
#include "MultiScreenController.h"
#include "ScreenController.h"
#include "types.h"


BOOST_AUTO_TEST_CASE(testUniformScreenStates)
{
    auto mockController1 = new MockScreenController(ScreenState::ON);
    auto mockController2 = new MockScreenController(ScreenState::ON);
    std::vector<std::unique_ptr<ScreenController>> controllers;

    controllers.push_back(std::unique_ptr<ScreenController>(mockController1));
    controllers.push_back(std::unique_ptr<ScreenController>(mockController2));

    auto multiController = new MultiScreenController(std::move(controllers));
    multiController->getState();
    BOOST_CHECK_EQUAL(multiController->getState(), ScreenState::ON);
}

BOOST_AUTO_TEST_CASE(testDifferentScreenStates)
{
    auto mockControllerOn = new MockScreenController(ScreenState::ON);
    auto mockControllerOff = new MockScreenController(ScreenState::OFF);
    std::vector<std::unique_ptr<ScreenController>> controllers;

    controllers.push_back(std::unique_ptr<ScreenController>(mockControllerOn));
    controllers.push_back(std::unique_ptr<ScreenController>(mockControllerOff));

    auto multiController = new MultiScreenController(std::move(controllers));
    multiController->getState();

    BOOST_CHECK_EQUAL(multiController->getState(), ScreenState::UNDEF);

}

BOOST_AUTO_TEST_CASE(testSignals)
{
    auto mockController = new MockScreenController(ScreenState::ON);
    std::vector<std::unique_ptr<ScreenController>> controllers;
    controllers.push_back(std::unique_ptr<ScreenController>(mockController));

    auto multiController = new MultiScreenController(std::move(controllers));

    multiController->checkPowerState();
    BOOST_CHECK_EQUAL(multiController->getState(), ScreenState::ON);

    multiController->powerOff();
    BOOST_CHECK_EQUAL(multiController->getState(), ScreenState::OFF);

    multiController->powerOn();
    BOOST_CHECK_EQUAL(multiController->getState(), ScreenState::ON);
}
