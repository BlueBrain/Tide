/*********************************************************************/
/* Copyright (c) 2014-2015, EPFL/Blue Brain Project                  */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
/*                     Daniel.Nachbaur@epfl.ch                       */
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

#define BOOST_TEST_MODULE ContentWindowControllerTests
#include <boost/test/unit_test.hpp>

#include <boost/make_shared.hpp>

#include "ContentWindow.h"
#include "control/ContentWindowController.h"
#include "control/LayoutEngine.h"
#include "DisplayGroup.h"

#include "MinimalGlobalQtApp.h"
BOOST_GLOBAL_FIXTURE( MinimalGlobalQtApp );

#include "DummyContent.h"

namespace
{
const QSizeF wallSize( 1000, 1000 );
const QSize CONTENT_SIZE( 800, 600 );
const QSize BIG_CONTENT_SIZE( CONTENT_SIZE * (Content::getMaxScale()+1) );
const QSize SMALL_CONTENT_SIZE( CONTENT_SIZE / 4 );
const qreal CONTENT_AR = qreal(CONTENT_SIZE.width()) /
                         qreal(CONTENT_SIZE.height());
}

BOOST_AUTO_TEST_CASE( testResizeAndMove )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( CONTENT_SIZE );
    ContentWindow window( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( window, *displayGroup );

    const QPointF targetPosition( 124.2, 457.3 );
    const QSizeF targetSize( 0.7 * QSizeF( content->getDimensions( )));

    controller.moveTo( targetPosition );
    controller.resize( targetSize );

    const QRectF& coords = window.getCoordinates();

    BOOST_CHECK_EQUAL( coords.topLeft(), targetPosition );
    BOOST_CHECK_EQUAL( coords.size(), targetSize );

    const QPointF targetCenterPosition( 568.2, 389.0 );
    controller.moveCenterTo( targetCenterPosition );

    BOOST_CHECK_EQUAL( coords.center(), targetCenterPosition );
    BOOST_CHECK_EQUAL( coords.size(), targetSize );

    const QPointF fixedCenter = coords.center();
    const QSizeF centeredSize( 0.5 * QSizeF( content->getDimensions( )));

    controller.resize( centeredSize, WindowPoint::CENTER );

    BOOST_CHECK_CLOSE( coords.center().x(), fixedCenter.x(), 0.00001 );
    BOOST_CHECK_CLOSE( coords.center().y(), fixedCenter.y(), 0.00001 );
    BOOST_CHECK_CLOSE( coords.width(), centeredSize.width(), 0.00001 );
    BOOST_CHECK_CLOSE( coords.height(), centeredSize.height(), 0.00001 );
}

BOOST_AUTO_TEST_CASE( testScaleByPixelDelta )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( CONTENT_SIZE );
    ContentWindow window( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( window, *displayGroup );

    const QRectF& coords = window.getCoordinates();
    const auto pixelDelta = 40.0;

    controller.scale( QPointF(), pixelDelta );
    BOOST_CHECK_EQUAL( coords.height(), CONTENT_SIZE.height() + pixelDelta );
    BOOST_CHECK_EQUAL( coords.width(),
                       CONTENT_SIZE.width() + pixelDelta * CONTENT_AR );
    BOOST_CHECK_EQUAL( coords.x(), 0 );
    BOOST_CHECK_EQUAL( coords.y(), 0 );

    controller.scale( coords.bottomRight(), -pixelDelta );
    BOOST_CHECK_EQUAL( coords.size().width(), CONTENT_SIZE.width( ));
    BOOST_CHECK_EQUAL( coords.size().height(), CONTENT_SIZE.height( ));
    BOOST_CHECK_EQUAL( coords.y(), pixelDelta );
    BOOST_CHECK_CLOSE( coords.x(), pixelDelta * CONTENT_AR, 0.00001 );
}

ContentWindowPtr makeDummyWindow()
{
    ContentPtr content( new DummyContent );
    content->setDimensions( CONTENT_SIZE );
    ContentWindowPtr window = boost::make_shared<ContentWindow>( content );
    window->setCoordinates( QRectF( 610, 220, 30, 40 ));

    const QRectF& coords = window->getCoordinates();
    BOOST_REQUIRE_EQUAL( coords.topLeft(), QPointF( 610, 220 ));
    BOOST_REQUIRE_EQUAL( coords.center(), QPointF( 625, 240 ));

    return window;
}

BOOST_AUTO_TEST_CASE( testOneToOneSize )
{
    ContentWindowPtr window = makeDummyWindow();
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( *window, *displayGroup );
    const QRectF& coords = window->getCoordinates();

    controller.adjustSize( SIZE_1TO1 );

    // 1:1 size restored around existing window center
    BOOST_CHECK_EQUAL( coords.size(), CONTENT_SIZE );
    BOOST_CHECK_EQUAL( coords.center(), QPointF( 625, 240 ));
}

BOOST_AUTO_TEST_CASE( testOneToOneFittingSize )
{
    ContentWindowPtr window = makeDummyWindow();
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( *window, *displayGroup );
    const QRectF& coords = window->getCoordinates();

    controller.adjustSize( SIZE_1TO1_FITTING );

    // 1:1 size restored around existing window center
    BOOST_CHECK_EQUAL( coords.size(), CONTENT_SIZE );
    BOOST_CHECK_EQUAL( coords.center(), QPointF( 625, 240 ));

    // Big content constrained to 0.9 * wallSize
    window->getContent()->setDimensions( 2 * wallSize.toSize( ));
    controller.adjustSize( SIZE_1TO1_FITTING );
    BOOST_CHECK_EQUAL( coords.size(), 0.9 * wallSize );
    BOOST_CHECK_EQUAL( coords.center(), QPointF( 625, 240 ));
}

BOOST_AUTO_TEST_CASE( testSizeLimitsBigContent )
{
    ContentWindowPtr window = makeDummyWindow();
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( *window, *displayGroup );

    // Make a large content and validate it
    ContentPtr content = window->getContent();
    content->setDimensions( BIG_CONTENT_SIZE );
    BOOST_REQUIRE_EQUAL( Content::getMaxScale(), 3.0 );
    BOOST_REQUIRE_EQUAL( content->getMaxDimensions(),
                         BIG_CONTENT_SIZE * Content::getMaxScale( ));

    // Test controller and zoom limits
    BOOST_CHECK_EQUAL( controller.getMinSize(), QSize( 300, 300 ));
    BOOST_CHECK_EQUAL( controller.getMaxSize(),
                       BIG_CONTENT_SIZE * Content::getMaxScale( ));

    const QSizeF normalMaxSize = controller.getMaxSize();
    window->getContent()->setZoomRect( QRectF( QPointF( 0.3, 0.1 ),
                                               QSizeF( 0.25, 0.25 )));
    BOOST_CHECK_EQUAL( controller.getMaxSize(), 0.25 * normalMaxSize );
}

BOOST_AUTO_TEST_CASE( testSizeLimitsSmallContent )
{
    ContentWindowPtr window = makeDummyWindow();
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( *window, *displayGroup );

    // Make a small content and validate it
    ContentPtr content = window->getContent();
    content->setDimensions( SMALL_CONTENT_SIZE );
    BOOST_REQUIRE_EQUAL( content->getMaxDimensions(),
                         SMALL_CONTENT_SIZE * Content::getMaxScale( ));
    BOOST_REQUIRE_EQUAL( Content::getMaxScale(), 3.0 );

    // Test controller and zoom limits
    BOOST_CHECK_EQUAL( controller.getMinSize(), QSize( 300, 300 ));
    BOOST_CHECK_EQUAL( controller.getMaxSize(),
                       SMALL_CONTENT_SIZE * Content::getMaxScale( ));

    const QSizeF normalMaxSize = controller.getMaxSize();
    window->getContent()->setZoomRect( QRectF( QPointF( 0.3, 0.1 ),
                                               QSizeF( 0.25, 0.25 )));
    BOOST_CHECK_EQUAL( controller.getMaxSize(), 0.25 * normalMaxSize );
}

BOOST_AUTO_TEST_CASE( testSizeHints )
{
    ContentWindowPtr window = makeDummyWindow();
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( *window, *displayGroup );
    ContentPtr content = window->getContent();

    const QSize maxSize( CONTENT_SIZE * 2 );
    const QSize minSize( CONTENT_SIZE / 2 );
    deflect::SizeHints hints;
    hints.maxWidth = maxSize.width();
    hints.maxHeight = maxSize.height();
    hints.minWidth = minSize.width();
    hints.minHeight = minSize.height();
    hints.preferredWidth = CONTENT_SIZE.width();
    hints.preferredHeight = CONTENT_SIZE.height();
    content->setSizeHints( hints );
    content->setDimensions( CONTENT_SIZE );
    const QRectF& coords = window->getCoordinates();

    // too big, constrains to maxSize
    controller.resize( maxSize * 2, CENTER );
    BOOST_CHECK_EQUAL( coords.size(), maxSize );

    // go back to preferred size
    controller.adjustSize( SIZE_1TO1 );
    BOOST_CHECK_EQUAL( coords.size(), CONTENT_SIZE );

    // perfect max size
    controller.resize( maxSize, CENTER );
    BOOST_CHECK_EQUAL( coords.size(), maxSize );

    // too small, clamped to minSize
    controller.resize( minSize / 2, CENTER );
    BOOST_CHECK_EQUAL( coords.size(), minSize );
}

BOOST_AUTO_TEST_CASE( testLargeSize )
{
    ContentWindowPtr window = makeDummyWindow();
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( *window, *displayGroup );
    const QRectF& coords = window->getCoordinates();

    controller.adjustSize( SIZE_LARGE );

    // 75% of the screen, resized around window center
    BOOST_CHECK_EQUAL( coords.center(), QPointF( 625, 240 ));
    BOOST_CHECK_EQUAL( coords.width(), 0.75 * wallSize.width( ));
    BOOST_CHECK_EQUAL( coords.height(), 0.75 * wallSize.width() / CONTENT_AR );
}


void _checkFullscreen( const QRectF& coords )
{
    // full screen, center on wall
    BOOST_CHECK_EQUAL( coords.x(), 0.0 );
    BOOST_CHECK_EQUAL( coords.y(), 125.0 );
    BOOST_CHECK_EQUAL( coords.width(), wallSize.width( ));
    BOOST_CHECK_EQUAL( coords.height(), wallSize.width() / CONTENT_AR );
}

void _checkFullscreenMax( const QRectF& coords )
{
    // full screen maximized, centered on wall
    BOOST_CHECK_CLOSE( coords.x(), -166.66666, 0.00001 );
    BOOST_CHECK_EQUAL( coords.y(), 0.0 );
    BOOST_CHECK_EQUAL( coords.width(), wallSize.height() * CONTENT_AR );
    BOOST_CHECK_EQUAL( coords.height(), wallSize.height( ));
}

BOOST_AUTO_TEST_CASE( testFullScreenSize )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( CONTENT_SIZE );
    content->setZoomRect( QRectF( 0.25, 0.25, 0.5, 0.5 ));
    ContentWindow window( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( window, *displayGroup );

    controller.adjustSize( SIZE_FULLSCREEN );
    _checkFullscreen( window.getCoordinates( ));
    // zoom reset
    BOOST_CHECK_EQUAL( content->getZoomRect(), UNIT_RECTF );
}

BOOST_AUTO_TEST_CASE( testFullScreenMaxSize )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( CONTENT_SIZE );
    content->setZoomRect( QRectF( 0.25, 0.25, 0.5, 0.5 ));
    ContentWindow window( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( window, *displayGroup );

    controller.adjustSize( SIZE_FULLSCREEN_MAX );
    _checkFullscreenMax( window.getCoordinates( ));
    // zoom reset
    BOOST_CHECK_EQUAL( content->getZoomRect(), UNIT_RECTF );
}

BOOST_AUTO_TEST_CASE( testToggleFullScreenMaxSize )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( CONTENT_SIZE );
    content->setZoomRect( QRectF( 0.25, 0.25, 0.5, 0.5 ));
    ContentWindow window( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( window, *displayGroup );

    // No effect if window is not in fullscreen
    const auto originalCoord = window.getDisplayCoordinates();
    controller.toogleFullscreenMaxSize();
    BOOST_CHECK_EQUAL( originalCoord, window.getDisplayCoordinates( ));

    // Toggle between two modes when in fullscreen
    window.setMode( ContentWindow::WindowMode::FULLSCREEN );
    controller.toogleFullscreenMaxSize();
    _checkFullscreenMax( window.getFullscreenCoordinates( ));
    controller.toogleFullscreenMaxSize();
    _checkFullscreen( window.getFullscreenCoordinates( ));
    controller.toogleFullscreenMaxSize();
    _checkFullscreenMax( window.getFullscreenCoordinates( ));
    controller.toogleFullscreenMaxSize();
    _checkFullscreen( window.getFullscreenCoordinates( ));
}

BOOST_AUTO_TEST_CASE( testResizeAndMoveInFullScreenMode )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( CONTENT_SIZE );
    content->setZoomRect( QRectF( 0.25, 0.25, 0.5, 0.5 ));
    ContentWindow window( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( window, *displayGroup );

    window.setMode( ContentWindow::WindowMode::FULLSCREEN );
    controller.adjustSize( SIZE_FULLSCREEN );
    _checkFullscreen( window.getDisplayCoordinates( ));

    // Window cannot move or scale down while in "FULLSCREEN"
    controller.moveBy( { 10.0, 20.0 });
    controller.scale( window.getDisplayCoordinates().center(), -15.0 );
    _checkFullscreen( window.getDisplayCoordinates( ));

    // Window can only move along the direction where it exceeds the displaywall
    controller.adjustSize( SIZE_FULLSCREEN_MAX );
    _checkFullscreenMax( window.getDisplayCoordinates( ));
    controller.moveBy( { 100.0, 100.0 });
    const auto& coords = window.getDisplayCoordinates();
    BOOST_CHECK_CLOSE( coords.x(), -66.66666, 0.0001 );
    BOOST_CHECK_EQUAL( coords.y(), 0.0 );
    controller.moveBy( { 100.0, 100.0 });
    BOOST_CHECK_EQUAL( coords.x(), 0.0 );
    BOOST_CHECK_EQUAL( coords.y(), 0.0 );

    // Window goes back to "FULLSCREEN" coordinates when scaling it down
    controller.scale( window.getDisplayCoordinates().center(), -500.0 );
    _checkFullscreen( window.getDisplayCoordinates( ));
}

BOOST_AUTO_TEST_CASE( testResizeRelativeToBorder )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( CONTENT_SIZE );
    ContentWindow window( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( window, *displayGroup );

    const QRectF originalCoords = window.getCoordinates();

    controller.resizeRelative( QPointF( 5, 5 ));
    BOOST_CHECK( window.getCoordinates() == originalCoords );

    // These border resize conserve the window aspect ratio
    window.setActiveHandle( ContentWindow::TOP );
    controller.resizeRelative( QPointF( 5, 5 ));
    BOOST_CHECK_EQUAL( window.getCoordinates().top() - 5, originalCoords.top());
    BOOST_CHECK_EQUAL( window.getCoordinates().width() + 5.0 * CONTENT_AR,
                       originalCoords.width());

    window.setActiveHandle( ContentWindow::BOTTOM );
    controller.resizeRelative( QPointF( 2, 2 ));
    BOOST_CHECK_EQUAL( window.getCoordinates().bottom() - 2,
                       originalCoords.bottom( ));
    BOOST_CHECK_EQUAL( window.getCoordinates().width() + 3.0 * CONTENT_AR,
                       originalCoords.width());
}

BOOST_AUTO_TEST_CASE( testResizeRelativeToCorner )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( CONTENT_SIZE );
    ContentWindow window( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    ContentWindowController controller( window, *displayGroup );

    const QRectF originalCoords = window.getCoordinates();

    // These corner resize alters the window aspect ratio
    static_cast<DummyContent*>( content.get( ))->fixedAspectRatio = false;
    BOOST_REQUIRE( window.setResizePolicy( ContentWindow::ADJUST_CONTENT ));
    window.setActiveHandle( ContentWindow::TOP_RIGHT );
    controller.resizeRelative( QPointF( 2, 10 ));
    BOOST_CHECK_EQUAL( window.getCoordinates().top() - 10,
                       originalCoords.top());
    BOOST_CHECK_EQUAL( window.getCoordinates().right() - 2,
                       originalCoords.right());
    BOOST_CHECK_EQUAL( window.getCoordinates().height() + 10,
                       originalCoords.height());
    BOOST_CHECK_EQUAL( window.getCoordinates().width() - 2,
                       originalCoords.width());

    const QRectF prevCoords = window.getCoordinates();

    window.setActiveHandle( ContentWindow::BOTTOM_LEFT );
    controller.resizeRelative( QPointF( 1, 2 ));
    BOOST_CHECK_EQUAL( window.getCoordinates().bottom() - 2,
                       prevCoords.bottom());
    BOOST_CHECK_EQUAL( window.getCoordinates().left() - 1,
                       prevCoords.left());
    BOOST_CHECK_EQUAL( window.getCoordinates().height() - 2,
                       prevCoords.height());
    BOOST_CHECK_EQUAL( window.getCoordinates().width() + 1,
                       prevCoords.width());
}

BOOST_AUTO_TEST_CASE( testLayoutEngineOneWindow )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( CONTENT_SIZE );
    ContentWindowPtr window = boost::make_shared<ContentWindow>( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    displayGroup->addContentWindow( window );
    displayGroup->addFocusedWindow( window );

    LayoutEngine engine( *displayGroup );
    const QRectF coords = engine.getFocusedCoord( *window );

    const qreal expectedPadding = 0.02 * wallSize.width() +
                                  0.3 * 0.3 * wallSize.height();
    const qreal expectedXOffset = 200.0; // window controls width
    const qreal totalExpectedMargin = 2.0 * expectedPadding + expectedXOffset;
    const qreal expectedWidth = wallSize.width() - totalExpectedMargin;
    const qreal expectedHeight = expectedWidth / content->getAspectRatio();
    const qreal expectedY = ( wallSize.height() - expectedHeight ) / 2.0;

    // focus mode, vertically centered on wall and repects inner margin
    BOOST_CHECK_EQUAL( coords.x(), expectedPadding + expectedXOffset );
    BOOST_CHECK_EQUAL( coords.y(), expectedY );
    BOOST_CHECK_EQUAL( coords.width(), expectedWidth );
    BOOST_CHECK_EQUAL( coords.height(), expectedHeight );
}

BOOST_AUTO_TEST_CASE( testLayoutEngineTwoWindows )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( CONTENT_SIZE );
    ContentWindowPtr window1 = boost::make_shared<ContentWindow>( content );
    ContentWindowPtr window2 = boost::make_shared<ContentWindow>( content );

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    displayGroup->addContentWindow( window1 );
    displayGroup->addContentWindow( window2 );
    displayGroup->addFocusedWindow( window1 );
    displayGroup->addFocusedWindow( window2 );

    LayoutEngine engine( *displayGroup );
    const QRectF coords1 = engine.getFocusedCoord( *window1 );
    const QRectF coords2 = engine.getFocusedCoord( *window2 );

    BOOST_CHECK_EQUAL( coords1, QRectF( 310, 443.75, 150, 112.5 ));
    BOOST_CHECK_EQUAL( coords2, QRectF( 740, 443.75, 150, 112.5 ));
}
