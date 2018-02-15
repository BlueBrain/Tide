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

#define BOOST_TEST_MODULE JsonBackgroundTests

#include <boost/test/unit_test.hpp>

#include "scene/Background.h"
#include "serialization/utils.h"
#include "json/serialization.h"
#include "json/templates.h"

namespace
{
const std::string defaultJson{
    R"({
    "color": "#000000",
    "uri": ""
}
)"};

const std::string notJson{"I'm not a Json string %#&"};

const std::string uriChangedJson{
    R"({
    "uri": "wall.png"
})"};

const std::string invalidBackgroundParametersJson{
    R"({
    "color": "#aeiouy",
    "uri": "notAValidFile.txt"
})"};

const std::string allValuesChangedJson{
    R"({
    "color": "#aa3300",
    "uri": "wall.png"
}
)"};
}

inline std::ostream& operator<<(std::ostream& str, const QUuid& uuid)
{
    str << uuid.toString();
    return str;
}

struct Fixture
{
    BackgroundPtr background = Background::create();
    QUuid uuid = background->getContentUUID();
};

BOOST_FIXTURE_TEST_CASE(testDefaultConstructor, Fixture)
{
    BOOST_CHECK_EQUAL(background->getColor().name(), "#000000");
    BOOST_CHECK_EQUAL(background->getUri(), "");
    BOOST_CHECK(background->getContent() == nullptr);
    BOOST_CHECK(!uuid.isNull());
}

BOOST_FIXTURE_TEST_CASE(testNotifications, Fixture)
{
    bool notified = false;
    QObject::connect(background.get(), &Background::updated,
                     [&](BackgroundPtr emitter) {
                         BOOST_CHECK_EQUAL(emitter, background);
                         notified = true;
                     });

    background->setColor(Qt::blue);
    BOOST_CHECK(notified);
    BOOST_CHECK_EQUAL(background->getColor().name(), QColor{Qt::blue}.name());

    notified = false;
    background->setUri("wall.png");
    BOOST_CHECK(notified);
    BOOST_CHECK_EQUAL(background->getUri(), "wall.png");
}

BOOST_FIXTURE_TEST_CASE(testBinarySerialization, Fixture)
{
    background->setColor(Qt::red);
    background->setUri("wall.png");
    const auto copy = serialization::binaryCopy(background);

    BOOST_CHECK_EQUAL(background->getColor().name(), copy->getColor().name());
    BOOST_CHECK_EQUAL(background->getUri(), copy->getUri());
    BOOST_CHECK_EQUAL(background->getContentUUID(), copy->getContentUUID());
    BOOST_REQUIRE(copy->getContent());
    BOOST_CHECK_EQUAL(copy->getContent()->parent(), copy.get());
    BOOST_CHECK_NE(background->getContent(), copy->getContent());
    BOOST_CHECK_EQUAL(background->getContent()->getURI(),
                      copy->getContent()->getURI());
}

BOOST_FIXTURE_TEST_CASE(testJsonSerialization, Fixture)
{
    BOOST_CHECK_EQUAL(json::dump(*background), defaultJson);
}

BOOST_FIXTURE_TEST_CASE(testInvalidJsonHasNoEffect, Fixture)
{
    BOOST_CHECK(!json::deserialize(notJson, *background));
    BOOST_CHECK_EQUAL(json::dump(*background), defaultJson);
    BOOST_CHECK_EQUAL(background->getContentUUID(), uuid);
}

BOOST_FIXTURE_TEST_CASE(testInvalidBackgroundParametersHaveNoEffect, Fixture)
{
    BOOST_CHECK(
        !json::deserialize(invalidBackgroundParametersJson, *background));
    BOOST_CHECK_EQUAL(background->getColor().name(), "#000000");
    BOOST_CHECK_EQUAL(background->getUri(), "");
    BOOST_CHECK_EQUAL(background->getContentUUID(), uuid);
}

BOOST_FIXTURE_TEST_CASE(testChangeOnlyUriFromJson, Fixture)
{
    BOOST_REQUIRE_EQUAL(background->getUri(), "");
    BOOST_CHECK(json::deserialize(uriChangedJson, *background));
    BOOST_CHECK_EQUAL(background->getUri(), "wall.png");
    BOOST_CHECK_NE(background->getContentUUID(), uuid);
}

BOOST_FIXTURE_TEST_CASE(testChangeAllValuesFromJson, Fixture)
{
    BOOST_CHECK(json::deserialize(allValuesChangedJson, *background));
    BOOST_CHECK_EQUAL(json::dump(*background), allValuesChangedJson);
    BOOST_CHECK_EQUAL(background->getColor().name(), "#aa3300");
    BOOST_CHECK_EQUAL(background->getUri(), "wall.png");
    BOOST_CHECK_NE(background->getContentUUID(), uuid);
}
