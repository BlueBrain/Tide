/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
/*                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>     */
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

#define BOOST_TEST_MODULE WebbrowserContent

#include <boost/test/unit_test.hpp>

#include "scene/WebbrowserContent.h"
#include "scene/WebbrowserHistory.h"
#include "serialization/utils.h"

namespace
{
const WebbrowserHistory history{{"url1", "url2", "url3"}, 1};
}

BOOST_AUTO_TEST_CASE(testDefaultWebbrowserHistoryEmpty)
{
    const WebbrowserHistory emptyHistory{};
    BOOST_CHECK_EQUAL(emptyHistory.currentItemIndex(), 0);
    BOOST_CHECK_EQUAL(emptyHistory.currentItem().toStdString(), "");
    BOOST_CHECK_EQUAL(emptyHistory.items().size(), 0);
}

BOOST_AUTO_TEST_CASE(testWebbrowserHistory)
{
    BOOST_CHECK_EQUAL(history.currentItemIndex(), 1);
    BOOST_CHECK_EQUAL(history.currentItem().toStdString(), "url2");
    BOOST_REQUIRE_EQUAL(history.items().size(), 3);
    BOOST_CHECK_EQUAL(history.items()[0].toStdString(), "url1");
    BOOST_CHECK_EQUAL(history.items()[1].toStdString(), "url2");
    BOOST_CHECK_EQUAL(history.items()[2].toStdString(), "url3");
}

BOOST_AUTO_TEST_CASE(testDefaultState)
{
    const WebbrowserContent content{"Webbrowser_1"};

    BOOST_CHECK_EQUAL(content.getType(), CONTENT_TYPE_WEBBROWSER);
    BOOST_CHECK_EQUAL(content.getURI().toStdString(), "Webbrowser_1");
    BOOST_CHECK_EQUAL(content.getTitle().toStdString(), "Web Browser");

    BOOST_CHECK_EQUAL(content.hasFixedAspectRatio(), false);

#if TIDE_USE_QT5WEBKITWIDGETS
    BOOST_CHECK(!content.getQmlControls().isEmpty());
#else
    BOOST_CHECK(content.getQmlControls().isEmpty());
#endif

    BOOST_CHECK_EQUAL(content.getUrl().toStdString(), "");
    BOOST_CHECK_EQUAL(content.getPage(), 0);
    BOOST_CHECK_EQUAL(content.getPageCount(), 0);
    BOOST_CHECK_EQUAL(content.getRestPort(), 0);

#if TIDE_USE_QT5WEBKITWIDGETS
    BOOST_CHECK_EQUAL(content.getAddressBar()->getCursorPosition(), 0);
    BOOST_CHECK_EQUAL(content.getAddressBar()->getSelectionStart(), 0);
    BOOST_CHECK_EQUAL(content.getAddressBar()->getSelectionEnd(), 0);
    BOOST_CHECK_EQUAL(content.getAddressBar()->isFocused(), false);
#endif
}

BOOST_AUTO_TEST_CASE(testSetters)
{
    WebbrowserContent content{"Webbrowser_1"};

    content.setUrl("some_url");

#if TIDE_USE_QT5WEBKITWIDGETS
    content.getAddressBar()->setFocused(true);
    content.getAddressBar()->setSelectionStart(2);
    content.getAddressBar()->setCursorPosition(4);
    content.getAddressBar()->setSelectionEnd(6);
#endif

    BOOST_CHECK_EQUAL(content.getUrl().toStdString(), "some_url");

#if TIDE_USE_QT5WEBKITWIDGETS
    BOOST_CHECK_EQUAL(content.getAddressBar()->isFocused(), true);
    BOOST_CHECK_EQUAL(content.getAddressBar()->getCursorPosition(), 4);
    BOOST_CHECK_EQUAL(content.getAddressBar()->getSelectionStart(), 2);
    BOOST_CHECK_EQUAL(content.getAddressBar()->getSelectionEnd(), 6);
#endif
}

BOOST_AUTO_TEST_CASE(testDataSerialization)
{
    WebbrowserContent content{"Webbrowser_1"};
    content.setUrl("erase_me");

    const auto data =
        WebbrowserContent::serializeData(history, "Some Title", 12345);
    BOOST_CHECK(!data.isEmpty());
    content.parseData(data);

    BOOST_CHECK_EQUAL(content.getTitle().toStdString(),
                      "Web Browser - Some Title");
    BOOST_CHECK_EQUAL(content.getUrl().toStdString(), "url2");
    BOOST_CHECK_EQUAL(content.getPage(), 1);
    BOOST_CHECK_EQUAL(content.getPageCount(), 3);
    BOOST_CHECK_EQUAL(content.getRestPort(), 12345);
}

BOOST_AUTO_TEST_CASE(testBinarySerialization)
{
    using WebbrowserContentPtr = std::shared_ptr<WebbrowserContent>;

    WebbrowserContentPtr source{new WebbrowserContent{"Webbrowser_1"}};
    source->parseData(
        WebbrowserContent::serializeData(history, "Some Title", 12345));
    // Set url resets history
    source->setUrl("some_url");
    BOOST_CHECK_EQUAL(source->getPage(), 0);
    BOOST_CHECK_EQUAL(source->getPageCount(), 1);

#if TIDE_USE_QT5WEBKITWIDGETS
    source->getAddressBar()->setFocused(true);
    source->getAddressBar()->setSelectionStart(2);
    source->getAddressBar()->setCursorPosition(4);
    source->getAddressBar()->setSelectionEnd(6);
#endif

    const auto copy = serialization::binaryCopy(source);
    BOOST_REQUIRE(copy);
    const auto& content = *copy;

    BOOST_CHECK_EQUAL(content.getTitle().toStdString(),
                      "Web Browser - Some Title");
    BOOST_CHECK_EQUAL(content.getUrl().toStdString(), "some_url");

    BOOST_CHECK_EQUAL(content.getPage(), 0);
    BOOST_CHECK_EQUAL(content.getPageCount(), 1);

#if TIDE_USE_QT5WEBKITWIDGETS
    BOOST_CHECK_EQUAL(content.getAddressBar()->isFocused(), true);
    BOOST_CHECK_EQUAL(content.getAddressBar()->getCursorPosition(), 4);
    BOOST_CHECK_EQUAL(content.getAddressBar()->getSelectionStart(), 2);
    BOOST_CHECK_EQUAL(content.getAddressBar()->getSelectionEnd(), 6);
#endif
}
