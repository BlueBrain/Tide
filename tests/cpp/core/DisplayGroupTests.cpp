/*********************************************************************/
/* Copyright (c) 2015, EPFL/Blue Brain Project                       */
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

#define BOOST_TEST_MODULE DisplayGroupTests

#include <boost/test/unit_test.hpp>

#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"

#include "DummyContent.h"

namespace
{
const QSize wallSize(1000, 1000);
const int WIDTH = 512;
const int HEIGHT = 512;

ContentPtr makeDummyContent()
{
    return std::make_unique<DummyContent>(QSize{WIDTH, HEIGHT});
}
}

BOOST_AUTO_TEST_CASE(testFocusWindows)
{
    auto window = std::make_shared<ContentWindow>(makeDummyContent());
    auto panel = std::make_shared<ContentWindow>(makeDummyContent(),
                                                 ContentWindow::PANEL);

    BOOST_REQUIRE(!window->isFocused());
    BOOST_REQUIRE(!panel->isFocused());

    DisplayGroupPtr displayGroup(new DisplayGroup(wallSize));
    displayGroup->addContentWindow(window);
    displayGroup->addContentWindow(panel);
    displayGroup->addFocusedWindow(window);

    BOOST_CHECK(window->isFocused());
    BOOST_CHECK(!panel->isFocused());

    displayGroup->removeFocusedWindow(window);
    BOOST_CHECK(!window->isFocused());
}

BOOST_AUTO_TEST_CASE(testWindowZorder)
{
    auto window0 = std::make_shared<ContentWindow>(makeDummyContent());
    auto window1 = std::make_shared<ContentWindow>(makeDummyContent());
    auto window2 = std::make_shared<ContentWindow>(makeDummyContent());

    DisplayGroupPtr group = std::make_shared<DisplayGroup>(wallSize);
    BOOST_REQUIRE(group->isEmpty());

    BOOST_CHECK_EQUAL(group->getZindex(window0), -1);
    BOOST_CHECK_EQUAL(group->getZindex(window1), -1);
    BOOST_CHECK_EQUAL(group->getZindex(window2), -1);

    group->addContentWindow(window0);
    group->addContentWindow(window1);

    BOOST_CHECK_EQUAL(group->getZindex(window0), 0);
    BOOST_CHECK_EQUAL(group->getZindex(window1), 1);
    BOOST_CHECK_EQUAL(group->getZindex(window2), -1);

    group->addContentWindow(window2);

    BOOST_CHECK_EQUAL(group->getZindex(window2), 2);

    group->moveToFront(window1);
    BOOST_CHECK_EQUAL(group->getZindex(window0), 0);
    BOOST_CHECK_EQUAL(group->getZindex(window2), 1);
    BOOST_CHECK_EQUAL(group->getZindex(window1), 2);

    group->moveToFront(window0);
    BOOST_CHECK_EQUAL(group->getZindex(window2), 0);
    BOOST_CHECK_EQUAL(group->getZindex(window1), 1);
    BOOST_CHECK_EQUAL(group->getZindex(window0), 2);

    group->removeContentWindow(window1);
    BOOST_CHECK_EQUAL(group->getZindex(window2), 0);
    BOOST_CHECK_EQUAL(group->getZindex(window0), 1);
    BOOST_CHECK_EQUAL(group->getZindex(window1), -1);
}
