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
namespace ut = boost::unit_test;

#include "ContentWindow.h"
#include "DisplayGroup.h"

#include "DummyContent.h"

#include <boost/make_shared.hpp>

namespace
{
const QSize wallSize( 1000, 1000 );
const int WIDTH = 512;
const int HEIGHT = 512;
}

BOOST_AUTO_TEST_CASE( testControllerCreationByDisplayGroup )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindowPtr window = boost::make_shared<ContentWindow>( content );

    BOOST_CHECK( !window->getController( ));

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    displayGroup->addContentWindow( window );

    BOOST_CHECK( window->getController( ));
}

BOOST_AUTO_TEST_CASE( testFocusWindows )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindowPtr window = boost::make_shared<ContentWindow>( content );
    ContentWindowPtr panel =
            boost::make_shared<ContentWindow>( content, ContentWindow::PANEL );

    BOOST_REQUIRE( !window->isFocused( ));
    BOOST_REQUIRE( !panel->isFocused( ));

    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    displayGroup->addContentWindow( window );
    displayGroup->addContentWindow( panel );
    displayGroup->focus( window->getID( ));

    BOOST_CHECK( window->isFocused( ));
    BOOST_CHECK( !panel->isFocused( ));

    displayGroup->unfocus( window->getID( ));
    BOOST_CHECK( !window->isFocused( ));
}

BOOST_AUTO_TEST_CASE( testWindowZorder )
{
    ContentPtr content( new DummyContent );
    content->setDimensions( QSize( WIDTH, HEIGHT ));
    ContentWindowPtr window0 = boost::make_shared<ContentWindow>( content );
    ContentWindowPtr window1 = boost::make_shared<ContentWindow>( content );
    ContentWindowPtr window2 = boost::make_shared<ContentWindow>( content );

    DisplayGroupPtr group = boost::make_shared<DisplayGroup>( wallSize );
    BOOST_REQUIRE( group->isEmpty( ));

    BOOST_CHECK_EQUAL( group->getZindex( window0 ), -1 );
    BOOST_CHECK_EQUAL( group->getZindex( window1 ), -1 );
    BOOST_CHECK_EQUAL( group->getZindex( window2 ), -1 );

    group->addContentWindow( window0 );
    group->addContentWindow( window1 );

    BOOST_CHECK_EQUAL( group->getZindex( window0 ), 0 );
    BOOST_CHECK_EQUAL( group->getZindex( window1 ), 1 );
    BOOST_CHECK_EQUAL( group->getZindex( window2 ), -1 );

    group->addContentWindow( window2 );

    BOOST_CHECK_EQUAL( group->getZindex( window2 ), 2 );

    group->moveContentWindowToFront( window1 );
    BOOST_CHECK_EQUAL( group->getZindex( window0 ), 0 );
    BOOST_CHECK_EQUAL( group->getZindex( window2 ), 1 );
    BOOST_CHECK_EQUAL( group->getZindex( window1 ), 2 );

    group->moveContentWindowToFront( window0 );
    BOOST_CHECK_EQUAL( group->getZindex( window2 ), 0 );
    BOOST_CHECK_EQUAL( group->getZindex( window1 ), 1 );
    BOOST_CHECK_EQUAL( group->getZindex( window0 ), 2 );

    group->removeContentWindow( window1 );
    BOOST_CHECK_EQUAL( group->getZindex( window2 ), 0 );
    BOOST_CHECK_EQUAL( group->getZindex( window0 ), 1 );
    BOOST_CHECK_EQUAL( group->getZindex( window1 ), -1 );
}
