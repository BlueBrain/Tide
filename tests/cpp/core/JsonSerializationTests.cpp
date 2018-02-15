/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
/*                     Pawel Podhajski <pawel.podhajski@epfl.ch>     */
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

#define BOOST_TEST_MODULE JsonSerializationTests

#include <boost/test/unit_test.hpp>

#include "rest/serialization.h"
#include "scene/ContentFactory.h"
#include "scene/DisplayGroup.h"
// include last
#include "json/templates.h"

namespace
{
const QString imageUri{"wall.png"};
const QSize wallSize{1000, 1000};
}

BOOST_AUTO_TEST_CASE(testSerializeDisplayGroup)
{
    auto group = std::make_shared<DisplayGroup>(wallSize);
    auto content = ContentFactory::getContent(imageUri);
    const auto contentSize = content->getDimensions();
    auto contentWindow = std::make_shared<ContentWindow>(std::move(content));
    contentWindow->setCoordinates({QPointF{64, 79}, contentSize});
    group->addContentWindow(contentWindow);

    const auto serializedWindows = json::dump(*group);

    const auto object = json::parse(serializedWindows);
    BOOST_REQUIRE(object.contains("windows"));

    const auto windows = object.value("windows").toArray();
    BOOST_REQUIRE_EQUAL(windows.size(), 1);
    BOOST_REQUIRE(windows[0].isObject());

    const auto window = windows[0].toObject();
    BOOST_CHECK_EQUAL(window.value("aspectRatio").toDouble(), 2.0);
    BOOST_CHECK_EQUAL(window.value("height").toInt(), 128);
    BOOST_CHECK_EQUAL(window.value("width").toInt(), 256);
    BOOST_CHECK_EQUAL(window.value("minHeight").toInt(), 300);
    BOOST_CHECK_EQUAL(window.value("minWidth").toInt(), 600);
    BOOST_CHECK_EQUAL(window.value("focus").toBool(true), false);
    BOOST_CHECK_EQUAL(window.value("fullscreen").toBool(true), false);
    BOOST_CHECK_EQUAL(window.value("selected").toBool(true), false);
    BOOST_CHECK_EQUAL(window.value("mode").toInt(99), 0);
    BOOST_CHECK_EQUAL(window.value("title").toString(), imageUri);
    BOOST_CHECK_EQUAL(window.value("uri").toString(), imageUri);
    const auto uuid = QString("{%1}").arg(window.value("uuid").toString());
    BOOST_CHECK_EQUAL(uuid, contentWindow->getID().toString());
    BOOST_CHECK_EQUAL(window.value("x").toInt(), 64);
    BOOST_CHECK_EQUAL(window.value("y").toInt(), 79);
    BOOST_CHECK_EQUAL(window.value("z").toInt(99), 0);
}
