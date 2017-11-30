/*********************************************************************/
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project                  */
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

#define BOOST_TEST_MODULE JsonOptionsTests

#include <boost/test/unit_test.hpp>

#include "rest/serialization.h"
#include "scene/Options.h"

namespace
{
const std::string defaultJson{
    R"({
    "alphaBlending": false,
    "autoFocusStreamers": true,
    "clock": false,
    "contentTiles": false,
    "controlArea": true,
    "statistics": false,
    "testPattern": false,
    "touchPoints": true,
    "windowBorders": true,
    "windowTitles": true,
    "zoomContext": true
}
)"};

const std::string notJson{"I'm not a Json string %#&"};

const std::string uriChangedJson{
    R"({
    "testPattern": true
})"};

const std::string allValuesChangedJson{
    R"({
    "alphaBlending": true,
    "autoFocusStreamers": false,
    "clock": true,
    "contentTiles": true,
    "controlArea": false,
    "statistics": true,
    "testPattern": true,
    "touchPoints": false,
    "windowBorders": false,
    "windowTitles": false,
    "zoomContext": false
}
)"};
}

struct Fixture
{
    OptionsPtr options = Options::create();
};

BOOST_FIXTURE_TEST_CASE(testDefaultSerialization, Fixture)
{
    BOOST_CHECK_EQUAL(to_json(*options), defaultJson);
}

BOOST_FIXTURE_TEST_CASE(testInvalidJsonHasNoEffect, Fixture)
{
    BOOST_CHECK(!from_json(*options, notJson));
    BOOST_CHECK_EQUAL(to_json(*options), defaultJson);
}

BOOST_FIXTURE_TEST_CASE(testChangeSingleValueFromJson, Fixture)
{
    BOOST_REQUIRE_EQUAL(options->getShowTestPattern(), false);
    BOOST_CHECK(from_json(*options, uriChangedJson));
    BOOST_CHECK_EQUAL(options->getShowTestPattern(), true);
}

BOOST_FIXTURE_TEST_CASE(testChangeAllValuesFromJson, Fixture)
{
    BOOST_CHECK(from_json(*options, allValuesChangedJson));
    BOOST_CHECK_EQUAL(to_json(*options), allValuesChangedJson);
}
