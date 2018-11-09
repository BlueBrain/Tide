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

#define BOOST_TEST_MODULE DisplayGroupTests

#include <boost/test/unit_test.hpp>

#include "scene/DisplayGroup.h"
#include "scene/Window.h"

#include "DummyContent.h"

namespace
{
const QSize wallSize(1000, 1000);
const int WIDTH = 512;
const int HEIGHT = 512;

ContentPtr makeDummyContent(const QString& uri = QString())
{
    return std::make_unique<DummyContent>(QSize{WIDTH, HEIGHT}, uri);
}
}

BOOST_AUTO_TEST_CASE(focus_window)
{
    auto window = std::make_shared<Window>(makeDummyContent());
    auto panel = std::make_shared<Window>(makeDummyContent(), Window::PANEL);

    BOOST_REQUIRE(!window->isFocused());
    BOOST_REQUIRE(!panel->isFocused());

    auto group = DisplayGroup::create(wallSize);
    group->add(window);
    group->add(panel);
    group->addFocusedWindow(window);

    BOOST_CHECK(window->isFocused());
    BOOST_CHECK(!panel->isFocused());

    group->removeFocusedWindow(window);
    BOOST_CHECK(!window->isFocused());
}

BOOST_AUTO_TEST_CASE(get_focusable_windows)
{
    auto window = std::make_shared<Window>(makeDummyContent());
    auto panel = std::make_shared<Window>(makeDummyContent(), Window::PANEL);
    auto group = DisplayGroup::create(wallSize);
    group->add(window);
    group->add(panel);

    const auto focusableWindows = group->getFocusableWindows();
    BOOST_REQUIRE_EQUAL(focusableWindows.size(), 1);
    BOOST_CHECK(focusableWindows.count(window));
}

BOOST_AUTO_TEST_CASE(window_z_order)
{
    auto window0 = std::make_shared<Window>(makeDummyContent());
    auto window1 = std::make_shared<Window>(makeDummyContent());
    auto window2 = std::make_shared<Window>(makeDummyContent());

    auto group = DisplayGroup::create(wallSize);
    BOOST_REQUIRE(group->isEmpty());

    BOOST_CHECK_EQUAL(group->getZindex(window0->getID()), -1);
    BOOST_CHECK_EQUAL(group->getZindex(window1->getID()), -1);
    BOOST_CHECK_EQUAL(group->getZindex(window2->getID()), -1);

    group->add(window0);
    group->add(window1);

    BOOST_CHECK_EQUAL(group->getZindex(window0->getID()), 0);
    BOOST_CHECK_EQUAL(group->getZindex(window1->getID()), 1);
    BOOST_CHECK_EQUAL(group->getZindex(window2->getID()), -1);

    group->add(window2);

    BOOST_CHECK_EQUAL(group->getZindex(window2->getID()), 2);

    group->moveToFront(window1);
    BOOST_CHECK_EQUAL(group->getZindex(window0->getID()), 0);
    BOOST_CHECK_EQUAL(group->getZindex(window2->getID()), 1);
    BOOST_CHECK_EQUAL(group->getZindex(window1->getID()), 2);

    group->moveToFront(window0);
    BOOST_CHECK_EQUAL(group->getZindex(window2->getID()), 0);
    BOOST_CHECK_EQUAL(group->getZindex(window1->getID()), 1);
    BOOST_CHECK_EQUAL(group->getZindex(window0->getID()), 2);

    group->remove(window1);
    BOOST_CHECK_EQUAL(group->getZindex(window2->getID()), 0);
    BOOST_CHECK_EQUAL(group->getZindex(window0->getID()), 1);
    BOOST_CHECK_EQUAL(group->getZindex(window1->getID()), -1);
}

BOOST_AUTO_TEST_CASE(find_window_by_filename)
{
    auto windowA = std::make_shared<Window>(makeDummyContent("aaa"));
    auto windowB = std::make_shared<Window>(makeDummyContent("bbb"));
    auto windowC = std::make_shared<Window>(makeDummyContent("ccc"));

    auto group = DisplayGroup::create(wallSize);

    BOOST_CHECK(!group->findWindow("aaa"));
    BOOST_CHECK(!group->findWindow("bbb"));
    BOOST_CHECK(!group->findWindow("ccc"));
    BOOST_CHECK(!group->findWindow("ddd"));

    group->add(windowA);
    group->add(windowB);
    group->add(windowC);

    BOOST_CHECK(group->findWindow("aaa"));
    BOOST_CHECK(group->findWindow("bbb"));
    BOOST_CHECK(group->findWindow("ccc"));
    BOOST_CHECK(!group->findWindow("ddd"));
}
