/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#define BOOST_TEST_MODULE ContentWindowTests
#include <boost/test/unit_test.hpp>
namespace ut = boost::unit_test;

#include "ContentWindow.h"

#include "MinimalGlobalQtApp.h"
BOOST_GLOBAL_FIXTURE( MinimalGlobalQtApp );

#include "DummyContent.h"

namespace
{
const int WIDTH = 512;
const int HEIGHT = 512;
}

BOOST_AUTO_TEST_CASE( testInitialSize )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    const QRectF& coords = window.getCoordinates();

    // default 1:1 size, left-corner at the origin
    BOOST_CHECK_EQUAL( coords, QRectF( 0.0, 0.0, WIDTH, HEIGHT ));
    BOOST_CHECK_EQUAL( window.getBorder(), ContentWindow::NOBORDER );
}

BOOST_AUTO_TEST_CASE( testValidID )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    BOOST_CHECK( window.getID() != QUuid());
}

BOOST_AUTO_TEST_CASE( testUniqueID )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));

    ContentWindow window1( content );
    BOOST_CHECK( window1.getID() != QUuid());

    ContentWindow window2( content );
    BOOST_CHECK( window2.getID() != QUuid());

    BOOST_CHECK( window1.getID() != window2.getID());
}

BOOST_AUTO_TEST_CASE( testSetContent )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));

    ContentWindow window( content );
    BOOST_CHECK_EQUAL( window.getContent(), content );

    ContentPtr secondContent( new DummyContent );
    secondContent->setDimensions( QSize( 2 * WIDTH, 3 * HEIGHT ));

    window.setContent( secondContent );
    BOOST_CHECK_EQUAL( window.getContent(), secondContent );

    const QRectF& coords = window.getCoordinates();

    // Dimensions are currently not modified by the change of content,
    // because the only use case is to set an error content of the same size.
    BOOST_CHECK_EQUAL( coords, QRectF( 0.0, 0.0, WIDTH, HEIGHT ));
}

BOOST_AUTO_TEST_CASE( testZoom )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    BOOST_REQUIRE( content->getZoomRect() == UNIT_RECTF );

    content->setZoomRect( QRectF( 0.2, 0.2, 0.7, 0.7 ));
    BOOST_CHECK( content->getZoomRect() == QRectF( 0.2, 0.2, 0.7, 0.7 ));
    content->setZoomRect( QRectF( -0.1, 0.2, 0.7, 0.7 ));
    BOOST_CHECK( content->getZoomRect() == QRectF( -0.1, 0.2, 0.7, 0.7 ));
    content->setZoomRect( QRectF( 0.1, 0.5, 2.0, 2.0 ));
    BOOST_CHECK( content->getZoomRect() == QRectF( 0.1, 0.5, 2.0, 2.0 ));
}

BOOST_AUTO_TEST_CASE( testWindowState )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    BOOST_REQUIRE_EQUAL( window.getState(), ContentWindow::NONE );
    BOOST_REQUIRE( !window.isHidden( ));
    BOOST_REQUIRE( !window.isMoving( ));
    BOOST_REQUIRE( !window.isResizing( ));
    BOOST_REQUIRE( !window.isSelected( ));

    window.setState( ContentWindow::SELECTED );
    BOOST_CHECK_EQUAL( window.getState(), ContentWindow::SELECTED );
    BOOST_CHECK( window.isSelected( ));

    window.setState( ContentWindow::MOVING );
    BOOST_CHECK_EQUAL( window.getState(), ContentWindow::MOVING );
    BOOST_CHECK( window.isMoving( ));

    window.setState( ContentWindow::RESIZING );
    BOOST_CHECK_EQUAL( window.getState(), ContentWindow::RESIZING );
    BOOST_CHECK( window.isResizing( ));

    window.setState( ContentWindow::HIDDEN );
    BOOST_CHECK_EQUAL( window.getState(), ContentWindow::HIDDEN );
    BOOST_CHECK( window.isHidden( ));
}

BOOST_AUTO_TEST_CASE( testToggleSelectedState )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow window( content );

    BOOST_REQUIRE_EQUAL( window.getState(), ContentWindow::NONE );

    window.toggleSelectedState();
    BOOST_CHECK_EQUAL( window.getState(), ContentWindow::SELECTED );

    window.toggleSelectedState();
    BOOST_CHECK_EQUAL( window.getState(), ContentWindow::NONE );

    window.setState( ContentWindow::MOVING );
    window.toggleSelectedState();
    BOOST_CHECK_EQUAL( window.getState(), ContentWindow::MOVING );

    window.setState( ContentWindow::RESIZING );
    window.toggleSelectedState();
    BOOST_CHECK_EQUAL( window.getState(), ContentWindow::RESIZING );

    window.setState( ContentWindow::HIDDEN );
    window.toggleSelectedState();
    BOOST_CHECK_EQUAL( window.getState(), ContentWindow::HIDDEN );
}

BOOST_AUTO_TEST_CASE( testWindowType )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindow defaultWindow( content );
    ContentWindow window( content, ContentWindow::DEFAULT );
    ContentWindow panel( content, ContentWindow::PANEL );

    BOOST_CHECK_EQUAL( defaultWindow.isPanel(), false );
    BOOST_CHECK_EQUAL( window.isPanel(), false );
    BOOST_CHECK_EQUAL( panel.isPanel(), true );
}
