/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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

#define BOOST_TEST_MODULE VisibilityHelperTests
#include <boost/test/unit_test.hpp>

#include "VisibilityHelper.h"
#include "scene/DisplayGroup.h"
#include "scene/Window.h"

#include "DummyContent.h"

namespace
{
const QSize groupSize(800, 600);
const QSize size(200, 200);
const QRect viewRect(0, 0, groupSize.width() / 2, groupSize.height());
const QRect centeredViewRect(350, 250, 100, 100);

ContentPtr makeDummyContent()
{
    return std::make_unique<DummyContent>(size);
}
}

struct Fixture
{
    Fixture()
    {
        group->add(window);
        BOOST_REQUIRE_EQUAL(window->getCoordinates(),
                            QRectF(QPointF(0, 0), size));
    }

    DisplayGroupPtr group{DisplayGroup::create(groupSize)};
    WindowPtr window{new Window{makeDummyContent()}};
    VisibilityHelper helper{*group, viewRect};
};

BOOST_FIXTURE_TEST_CASE(testSingleWindow, Fixture)
{
    const auto& coord = window->getCoordinates();

    // Fully inside
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window), coord);

    // Fully outside
    window->setCoordinates(QRectF(QPointF(400, 0), size));
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window), QRectF());

    // Half-inside horizontally
    window->setCoordinates(QRectF(QPointF(300, 0), size));
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window),
                      QRectF(QPointF(0, 0), QSize(100, 200)));

    window->setCoordinates(QRectF(QPointF(-100, 0), size));
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window),
                      QRectF(QPointF(100, 0), QSize(100, 200)));

    // Half-inside vertically, bottom cut
    window->setCoordinates(QRectF(QPointF(0, 500), size));
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window),
                      QRectF(QPointF(0, 0), QSize(200, 100)));

    // Half-inside vertically, top cut
    window->setCoordinates(QRectF(QPointF(0, -100), size));
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window),
                      QRectF(QPointF(0, 100), QSize(200, 100)));

    // Corner view cut
    window->setCoordinates(QRectF(QPointF(300, 500), size));
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window),
                      QRectF(QPointF(0, 0), QSize(100, 100)));
}

BOOST_FIXTURE_TEST_CASE(testSingleWindowCenteredViewRect, Fixture)
{
    window->setCoordinates(QRectF(QPointF(300, 200), size));

    VisibilityHelper otherViewRectHelper(*group, centeredViewRect);
    BOOST_CHECK_EQUAL(otherViewRectHelper.getVisibleArea(*window),
                      QRectF(QPointF(50, 50), centeredViewRect.size()));
}

BOOST_FIXTURE_TEST_CASE(testOverlappingWindow, Fixture)
{
    const auto& coord = window->getCoordinates();
    BOOST_REQUIRE_EQUAL(helper.getVisibleArea(*window), coord);

    auto otherWindow = std::make_shared<Window>(makeDummyContent());
    group->add(otherWindow);

    // Full overlap
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window), QRectF());

    // Focused
    window->setFocusedCoordinates(coord);
    window->setMode(Window::WindowMode::FOCUSED);
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window), coord);
    window->setMode(Window::WindowMode::STANDARD);

    // Half-above horizonally
    otherWindow->setCoordinates(QRectF(QPointF(100, 0), size));
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window),
                      QRectF(QPointF(0, 0), QSize(100, 200)));

    // Half-above vertically
    otherWindow->setCoordinates(QRectF(QPointF(0, 100), size));
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window),
                      QRectF(QPointF(0, 0), QSize(200, 100)));

    // Corner overlap (no cut)
    otherWindow->setCoordinates(QRectF(QPointF(100, 100), size));
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window),
                      QRectF(QPointF(0, 0), QSize(200, 200)));
}

BOOST_FIXTURE_TEST_CASE(testUnderlyingWindow, Fixture)
{
    const auto& coord = window->getCoordinates();

    auto otherWindow = std::make_shared<Window>(makeDummyContent());
    group->add(otherWindow);
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window), QRectF());
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*otherWindow), coord);

    group->moveToFront(window);
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*otherWindow), QRectF());
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window), coord);
}

BOOST_FIXTURE_TEST_CASE(testViewCutCombinedWithOverlappingWindow, Fixture)
{
    auto otherWindow = std::make_shared<Window>(makeDummyContent());
    group->add(otherWindow);

    // Corner view cut
    window->setCoordinates(QRectF(QPointF(300, 500), size));
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window),
                      QRectF(QPointF(0, 0), QSize(100, 100)));

    // Partial corner overlap (no cut)
    otherWindow->setCoordinates(QRectF(QPointF(150, 350), size));
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window),
                      QRectF(QPointF(0, 0), QSize(100, 100)));

    // Full corner overlap
    otherWindow->setCoordinates(QRectF(QPointF(200, 400), size));
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window), QRectF());
}

BOOST_FIXTURE_TEST_CASE(testFullscreenWindowOverlapEverything, Fixture)
{
    const auto& coord = window->getCoordinates();

    auto otherWindow = std::make_shared<Window>(makeDummyContent());
    group->add(otherWindow);
    BOOST_REQUIRE_EQUAL(helper.getVisibleArea(*window), QRectF());
    BOOST_REQUIRE_EQUAL(helper.getVisibleArea(*otherWindow), coord);

    // Even a "small" fullscreen window should overlap everything...
    window->setMode(Window::WindowMode::FULLSCREEN);
    group->setFullscreenWindow(window);
    const QRectF fullscreen(QPointF(), window->getCoordinates().size() / 2);
    window->setFullscreenCoordinates(fullscreen);
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*otherWindow), QRectF());
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window), fullscreen);

    // ...including focused windows
    auto focusWindow = std::make_shared<Window>(makeDummyContent());
    group->add(focusWindow);
    group->addFocusedWindow(focusWindow);
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*focusWindow), QRectF());
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window), fullscreen);
}

BOOST_FIXTURE_TEST_CASE(testHiddenWindowNotObstructingOthers, Fixture)
{
    const auto& coord = window->getCoordinates();

    auto otherWindow = std::make_shared<Window>(makeDummyContent());
    group->add(otherWindow);
    otherWindow->setState(Window::WindowState::HIDDEN);
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window), coord);

    otherWindow->setState(Window::WindowState::NONE);
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*window), QRectF());
    BOOST_CHECK_EQUAL(helper.getVisibleArea(*otherWindow), coord);
}
