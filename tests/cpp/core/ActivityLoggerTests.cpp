/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
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

#define BOOST_TEST_MODULE ActivityLoggerTests

#include <boost/test/unit_test.hpp>

#include "scene/DisplayGroup.h"
#include "tools/ActivityLogger.h"

#include "DummyContent.h"

#include <QObject>

namespace
{
const QSize wallSize(1000, 1000);

WindowPtr makeDummyWindow()
{
    return std::make_shared<Window>(ContentPtr{new DummyContent});
}
}

struct Fixture
{
    DisplayGroupPtr displayGroup{DisplayGroup::create(wallSize)};
    WindowPtr window1 = makeDummyWindow();
    WindowPtr window2 = makeDummyWindow();
    ActivityLogger logger;

    void connectWindowAddedSignal()
    {
        QObject::connect(displayGroup.get(), &DisplayGroup::windowAdded,
                         &logger, &ActivityLogger::logWindowAdded);
    }

    void connectWindowRemovedSignal()
    {
        QObject::connect(displayGroup.get(), &DisplayGroup::windowRemoved,
                         &logger, &ActivityLogger::logWindowRemoved);
    }
};

BOOST_FIXTURE_TEST_CASE(testAccumulatedWindowCount, Fixture)
{
    connectWindowAddedSignal();

    BOOST_REQUIRE_EQUAL(logger.getAccumulatedWindowCount(), 0);

    displayGroup->add(window1);
    BOOST_CHECK_EQUAL(logger.getAccumulatedWindowCount(), 1);

    displayGroup->add(window2);
    BOOST_CHECK_EQUAL(logger.getAccumulatedWindowCount(), 2);
}

BOOST_FIXTURE_TEST_CASE(testWindowCount, Fixture)
{
    connectWindowAddedSignal();
    connectWindowRemovedSignal();

    BOOST_REQUIRE_EQUAL(logger.getWindowCount(), 0);
    displayGroup->add(window1);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 1);
    displayGroup->remove(window1);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 0);
}

BOOST_FIXTURE_TEST_CASE(testWindowCountCannotGetBelow0, Fixture)
{
    connectWindowRemovedSignal();

    BOOST_REQUIRE_EQUAL(logger.getWindowCount(), 0);
    displayGroup->add(window1);
    displayGroup->remove(window1);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 0);
}

BOOST_FIXTURE_TEST_CASE(testWindowCountDoesNotIncludeHiddenWindows, Fixture)
{
    connectWindowAddedSignal();
    connectWindowRemovedSignal();

    BOOST_REQUIRE_EQUAL(logger.getWindowCount(), 0);

    window1->setState(Window::HIDDEN);
    displayGroup->add(window1);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 0);
    window1->setState(Window::NONE);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 1);

    window2->setState(Window::HIDDEN);
    displayGroup->add(window2);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 1);

    displayGroup->remove(window1);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 0);
    window2->setState(Window::NONE);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 1);
    window2->setState(Window::HIDDEN);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 0);
}

BOOST_FIXTURE_TEST_CASE(testInteractionCounter, Fixture)
{
    connectWindowAddedSignal();

    BOOST_REQUIRE_EQUAL(logger.getInteractionCount(), 0);

    displayGroup->add(window1);
    displayGroup->addFocusedWindow(window1);
    BOOST_CHECK_EQUAL(logger.getInteractionCount(), 2);
}

BOOST_FIXTURE_TEST_CASE(testLastInteractionName, Fixture)
{
    connectWindowAddedSignal();

    BOOST_REQUIRE_EQUAL(logger.getLastInteractionName(), "");

    displayGroup->add(window1);
    displayGroup->addFocusedWindow(window1);
    BOOST_CHECK_EQUAL(logger.getLastInteractionName(), "mode changed");
}

BOOST_FIXTURE_TEST_CASE(hideLauncher, Fixture)
{
    connectWindowAddedSignal();

    displayGroup->add(window1);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 1);
    window1->setState(Window::HIDDEN);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 0);
    window1->setState(Window::HIDDEN);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 0);
    window1->setState(Window::NONE);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 1);
    window1->setState(Window::MOVING);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 1);
}
