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

#define BOOST_TEST_MODULE JsonSerializationTests

#include <boost/test/unit_test.hpp>

#include "scene/ContentFactory.h"
#include "scene/DisplayGroup.h"
#include "tools/ActivityLogger.h"

#include "rest/serialization.h"
// include last
#include "json/templates.h"

#include "DummyContent.h"

#include <QRegularExpression>

namespace
{
const QString imageUri{"wall.png"};
const QSize wallSize{1000, 1000};

WindowPtr makeDummyWindow()
{
    return std::make_shared<Window>(ContentPtr{new DummyContent});
}

const std::string activityLoggerDefaultJson{
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
const QString activityLoggerRegexJson{
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

BOOST_AUTO_TEST_CASE(testSerializeDisplayGroup)
{
    auto group = DisplayGroup::create(wallSize);
    auto content = ContentFactory::getContent(imageUri);
    const auto contentSize = content->getDimensions();
    auto window = std::make_shared<Window>(std::move(content));
    window->setCoordinates({QPointF{64, 79}, contentSize});
    group->add(window);

    const auto serializedWindows = json::dump(*group);

    const auto object = json::parse(serializedWindows);
    BOOST_REQUIRE(object.contains("windows"));

    const auto windows = object.value("windows").toArray();
    BOOST_REQUIRE_EQUAL(windows.size(), 1);
    BOOST_REQUIRE(windows[0].isObject());

    const auto jsonWindow = windows[0].toObject();
    BOOST_CHECK_EQUAL(jsonWindow.value("aspectRatio").toDouble(), 2.0);
    BOOST_CHECK_EQUAL(jsonWindow.value("height").toInt(), 128);
    BOOST_CHECK_EQUAL(jsonWindow.value("width").toInt(), 256);
    BOOST_CHECK_EQUAL(jsonWindow.value("minHeight").toInt(), 300);
    BOOST_CHECK_EQUAL(jsonWindow.value("minWidth").toInt(), 600);
    BOOST_CHECK_EQUAL(jsonWindow.value("focus").toBool(true), false);
    BOOST_CHECK_EQUAL(jsonWindow.value("fullscreen").toBool(true), false);
    BOOST_CHECK_EQUAL(jsonWindow.value("selected").toBool(true), false);
    BOOST_CHECK_EQUAL(jsonWindow.value("mode").toInt(99), 0);
    BOOST_CHECK_EQUAL(jsonWindow.value("title").toString(), imageUri);
    BOOST_CHECK_EQUAL(jsonWindow.value("uri").toString(), imageUri);
    const auto uuid = QString("{%1}").arg(jsonWindow.value("uuid").toString());
    BOOST_CHECK_EQUAL(uuid, window->getID().toString());
    BOOST_CHECK_EQUAL(jsonWindow.value("x").toInt(), 64);
    BOOST_CHECK_EQUAL(jsonWindow.value("y").toInt(), 79);
}

BOOST_AUTO_TEST_CASE(testSerializeActivityLogger)
{
    ActivityLogger logger;
    BOOST_CHECK_EQUAL(json::dump(logger), activityLoggerDefaultJson);

    logger.logWindowAdded(makeDummyWindow());
    logger.logWindowAdded(makeDummyWindow());

    const auto regex = QRegularExpression(activityLoggerRegexJson);
    const auto json = QString::fromStdString(json::dump(logger));
    const auto matchedJson = regex.match(json).captured();
    BOOST_CHECK_EQUAL(json, matchedJson);
}
