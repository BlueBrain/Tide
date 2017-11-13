/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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

#define BOOST_TEST_MODULE LoggingUtilityTest

#include <boost/test/unit_test.hpp>

#include "LoggingUtility.h"
#include "rest/serialization.h"
#include "scene/DisplayGroup.h"

#include "DummyContent.h"

#include <QObject>
#include <QRegularExpression>

namespace
{
const QSize wallSize(1000, 1000);
const std::string defaultJson{
    R"({
    "event": {
        "count": 0,
        "last_event": "",
        "last_event_date": ""
    },
    "screens": {
        "last_change": "",
        "state": "UNDEF"
    },
    "window": {
        "accumulated_count": 0,
        "count": 0,
        "date_set": ""
    }
}
)"};
const QString regexJson{
    R"(\{
    "event": \{
        "count": 2,
        "last_event": "contentWindowAdded",
        "last_event_date": "\d{4}-\d{2}-\d{2}[A-Z]\d{2}:\d{2}:\d{2}.\d{6}"
    \},
    "screens": {
        "last_change": "",
        "state": "UNDEF"
    },
    "window": \{
        "accumulated_count": 2,
        "count": 2,
        "date_set": "\d{4}-\d{2}-\d{2}[A-Z]\d{2}:\d{2}:\d{2}.\d{6}"
    \}
\}
)"};
}

struct Fixture
{
    ContentPtr content{new DummyContent};
    DisplayGroupPtr displayGroup{new DisplayGroup(wallSize)};

    ContentWindowPtr window1 = std::make_shared<ContentWindow>(content);
    ContentWindowPtr window2 = std::make_shared<ContentWindow>(content);
    LoggingUtility logger;
};

BOOST_FIXTURE_TEST_CASE(testAccumulatedWindowCount, Fixture)
{
    QObject::connect(displayGroup.get(), &DisplayGroup::contentWindowAdded,
                     &logger, &LoggingUtility::logContentWindowAdded);

    BOOST_CHECK_EQUAL(logger.getAccumulatedWindowCount(), 0);

    displayGroup->addContentWindow(window1);
    BOOST_CHECK_EQUAL(logger.getAccumulatedWindowCount(), 1);

    displayGroup->addContentWindow(window2);
    BOOST_CHECK_EQUAL(logger.getAccumulatedWindowCount(), 2);
}

BOOST_FIXTURE_TEST_CASE(testWindowCount, Fixture)
{
    QObject::connect(displayGroup.get(), &DisplayGroup::contentWindowAdded,
                     &logger, &LoggingUtility::logContentWindowAdded);
    QObject::connect(displayGroup.get(), &DisplayGroup::contentWindowRemoved,
                     &logger, &LoggingUtility::logContentWindowRemoved);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 0);
    displayGroup->addContentWindow(window1);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 1);
    displayGroup->removeContentWindow(window1);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 0);
}

BOOST_FIXTURE_TEST_CASE(testWindowCountCannotGetBelow0, Fixture)
{
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 0);
    displayGroup->addContentWindow(window1);
    QObject::connect(displayGroup.get(), &DisplayGroup::contentWindowRemoved,
                     &logger, &LoggingUtility::logContentWindowRemoved);
    displayGroup->removeContentWindow(window1);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 0);
}

BOOST_FIXTURE_TEST_CASE(testWindowCountDoesNotIncludeHiddenWindows, Fixture)
{
    QObject::connect(displayGroup.get(), &DisplayGroup::contentWindowAdded,
                     &logger, &LoggingUtility::logContentWindowAdded);
    QObject::connect(displayGroup.get(), &DisplayGroup::contentWindowRemoved,
                     &logger, &LoggingUtility::logContentWindowRemoved);

    BOOST_REQUIRE_EQUAL(logger.getWindowCount(), 0);

    window1->setState(ContentWindow::HIDDEN);
    displayGroup->addContentWindow(window1);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 0);
    window1->setState(ContentWindow::NONE);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 1);

    window2->setState(ContentWindow::HIDDEN);
    displayGroup->addContentWindow(window2);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 1);

    displayGroup->removeContentWindow(window1);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 0);
    window2->setState(ContentWindow::NONE);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 1);
    window2->setState(ContentWindow::HIDDEN);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 0);
}

BOOST_FIXTURE_TEST_CASE(testInteractionCounter, Fixture)
{
    BOOST_CHECK_EQUAL(logger.getInteractionCount(), 0);

    QObject::connect(displayGroup.get(), &DisplayGroup::contentWindowAdded,
                     &logger, &LoggingUtility::logContentWindowAdded);

    displayGroup->addContentWindow(window1);
    displayGroup->addFocusedWindow(window1);
    BOOST_CHECK_EQUAL(logger.getInteractionCount(), 2);
}

BOOST_FIXTURE_TEST_CASE(testLastInteractionName, Fixture)
{
    BOOST_CHECK_EQUAL(logger.getLastInteractionName(), "");

    QObject::connect(displayGroup.get(), &DisplayGroup::contentWindowAdded,
                     &logger, &LoggingUtility::logContentWindowAdded);

    displayGroup->addContentWindow(window1);
    displayGroup->addFocusedWindow(window1);
    BOOST_CHECK_EQUAL(logger.getLastInteractionName(), "mode changed");
}

BOOST_FIXTURE_TEST_CASE(hideLauncher, Fixture)
{
    QObject::connect(displayGroup.get(), &DisplayGroup::contentWindowAdded,
                     &logger, &LoggingUtility::logContentWindowAdded);

    displayGroup->addContentWindow(window1);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 1);
    window1->setState(ContentWindow::HIDDEN);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 0);
    window1->setState(ContentWindow::HIDDEN);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 0);
    window1->setState(ContentWindow::NONE);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 1);
    window1->setState(ContentWindow::MOVING);
    BOOST_CHECK_EQUAL(logger.getWindowCount(), 1);
}

BOOST_FIXTURE_TEST_CASE(testJsonOutput, Fixture)
{
    BOOST_CHECK_EQUAL(to_json(logger), defaultJson);

    QObject::connect(displayGroup.get(), &DisplayGroup::contentWindowAdded,
                     &logger, &LoggingUtility::logContentWindowAdded);
    displayGroup->addContentWindow(window1);
    displayGroup->addContentWindow(window2);

    const auto regex = QRegularExpression(regexJson);
    const auto json = QString::fromStdString(to_json(logger));
    const auto matchedJson = regex.match(json).captured();
    BOOST_CHECK_EQUAL(json, matchedJson);
}
