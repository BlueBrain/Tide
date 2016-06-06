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
namespace ut = boost::unit_test;

#include "VisibilityHelper.h"
#include "DisplayGroup.h"
#include "ContentWindow.h"

#include "DummyContent.h"

#include <boost/make_shared.hpp>

namespace
{
const QSize groupSize( 800, 600 );
const QSize size( 200, 200 );
const QRect viewRect( 0, 0, groupSize.width() / 2, groupSize.height( ));
const QRect centeredViewRect( 350, 250, 100, 100 );
}

struct Fixture {
    Fixture()
        : group( new DisplayGroup( groupSize ))
        , content( new DummyContent )
        , helper( *group, viewRect )
    {
        content->setDimensions( size );
        window = boost::make_shared<ContentWindow>( content );
        group->addContentWindow( window );

        BOOST_REQUIRE_EQUAL( window->getCoordinates(),
                             QRectF( QPointF( 0, 0 ), size ));
    }

    DisplayGroupPtr group;
    ContentPtr content;
    ContentWindowPtr window;
    VisibilityHelper helper;
};

BOOST_FIXTURE_TEST_CASE( testSingleWindow, Fixture )
{
    const QRectF& coord = window->getCoordinates();

    // Fully inside
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ), coord );

    // Fully outside
    window->setCoordinates( QRectF( QPointF( 400, 0 ), size ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ), QRectF( ));

    // Half-inside horizontally
    window->setCoordinates( QRectF( QPointF( 300, 0 ), size ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ),
                       QRectF( QPointF( 0, 0 ), QSize( 100, 200 )));

    window->setCoordinates( QRectF( QPointF( -100, 0 ), size ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ),
                       QRectF( QPointF( 100, 0 ), QSize( 100, 200 )));

    // Half-inside vertically, bottom cut
    window->setCoordinates( QRectF( QPointF( 0, 500 ), size ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ),
                       QRectF( QPointF( 0, 0 ), QSize( 200, 100 )));

    // Half-inside vertically, top cut
    window->setCoordinates( QRectF( QPointF( 0, -100 ), size ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ),
                       QRectF( QPointF( 0, 100 ), QSize( 200, 100 )));

    // Corner view cut
    window->setCoordinates( QRectF( QPointF( 300, 500 ), size ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ),
                       QRectF( QPointF( 0, 0 ), QSize( 100, 100 )));
}

BOOST_FIXTURE_TEST_CASE( testSingleWindowCenteredViewRect, Fixture )
{
    window->setCoordinates( QRectF( QPointF( 300, 200 ), size ));

    VisibilityHelper otherViewRectHelper( *group, centeredViewRect );
    BOOST_CHECK_EQUAL( otherViewRectHelper.getVisibleArea( *window ),
                       QRectF( QPointF( 50, 50 ), centeredViewRect.size( )));
}

BOOST_FIXTURE_TEST_CASE( testOverlappingWindow, Fixture )
{
    const QRectF& coord = window->getCoordinates();
    BOOST_REQUIRE_EQUAL( helper.getVisibleArea( *window ), coord );

    ContentWindowPtr otherWindow = boost::make_shared<ContentWindow>( content );
    group->addContentWindow( otherWindow );

    // Full overlap
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ), QRectF( ));

    // Focused
    window->setFocusedCoordinates( coord );
    window->setMode( ContentWindow::WindowMode::FOCUSED );
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ), coord );
    window->setMode( ContentWindow::WindowMode::STANDARD );

    // Half-above horizonally
    otherWindow->setCoordinates( QRectF( QPointF( 100, 0 ), size ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ),
                       QRectF( QPointF( 0, 0 ), QSize( 100, 200 )));

    // Half-above vertically
    otherWindow->setCoordinates( QRectF( QPointF( 0, 100 ), size ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ),
                       QRectF( QPointF( 0, 0 ), QSize( 200, 100 )));

    // Corner overlap (no cut)
    otherWindow->setCoordinates( QRectF( QPointF( 100, 100 ), size ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ),
                       QRectF( QPointF( 0, 0 ), QSize( 200, 200 )));
}

BOOST_FIXTURE_TEST_CASE( testUnderlyingWindow, Fixture )
{
    const QRectF& coord = window->getCoordinates();

    ContentWindowPtr otherWindow = boost::make_shared<ContentWindow>( content );
    group->addContentWindow( otherWindow );
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ), QRectF( ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *otherWindow ), coord );

    group->moveContentWindowToFront( window );
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *otherWindow ), QRectF( ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ), coord );
}

BOOST_FIXTURE_TEST_CASE( testViewCutCombinedWithOverlappingWindow, Fixture )
{
    ContentWindowPtr otherWindow = boost::make_shared<ContentWindow>( content );
    group->addContentWindow( otherWindow );

    // Corner view cut
    window->setCoordinates( QRectF( QPointF( 300, 500 ), size ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ),
                       QRectF( QPointF( 0, 0 ), QSize( 100, 100 )));

    // Partial corner overlap (no cut)
    otherWindow->setCoordinates( QRectF( QPointF( 150, 350 ), size ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ),
                       QRectF( QPointF( 0, 0 ), QSize( 100, 100 )));

    // Full corner overlap
    otherWindow->setCoordinates( QRectF( QPointF( 200, 400 ), size ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ), QRectF( ));
}

BOOST_FIXTURE_TEST_CASE( testFullscreenWindowOverlapEverything, Fixture )
{
    const QRectF& coord = window->getCoordinates();

    ContentWindowPtr otherWindow = boost::make_shared<ContentWindow>( content );
    group->addContentWindow( otherWindow );
    BOOST_REQUIRE_EQUAL( helper.getVisibleArea( *window ), QRectF( ));
    BOOST_REQUIRE_EQUAL( helper.getVisibleArea( *otherWindow ), coord );

    // Even a "small" fullscreen window should overlap everything...
    group->showFullscreen( window->getID( ));
    const QRectF fullscreen( QPointF(), window->getCoordinates().size() / 2 );
    window->setFullscreenCoordinates( fullscreen );
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *otherWindow ), QRectF( ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ), fullscreen );

    // ...including focused windows
    ContentWindowPtr focusWindow = boost::make_shared<ContentWindow>( content );
    group->addContentWindow( focusWindow );
    group->focus( focusWindow->getID( ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *focusWindow ), QRectF( ));
    BOOST_CHECK_EQUAL( helper.getVisibleArea( *window ), fullscreen );
}
