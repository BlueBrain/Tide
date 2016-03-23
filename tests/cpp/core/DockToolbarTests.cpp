/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#define BOOST_TEST_MODULE DockToolbarTests
#include <boost/test/unit_test.hpp>
namespace ut = boost::unit_test;

#include "localstreamer/DockToolbar.h"
#include "types.h"

BOOST_AUTO_TEST_CASE( testDockToolbarImageSize )
{
    const QSize size( 512, 64 );
    DockToolbar toolbar( size );

    BOOST_CHECK_EQUAL( toolbar.getSize(), size );
    BOOST_CHECK_EQUAL( toolbar.getImage().size(), size );
}

BOOST_AUTO_TEST_CASE( testDockToolbarButtons )
{
    const QSize size( 512, 64 );
    DockToolbar toolbar( size );

    BOOST_CHECK( !toolbar.getButtonAt( QPoint( 128, 10 )));

    toolbar.addButton(new ToolbarButton("Button1", QImage(), "command1"));
    toolbar.addButton(new ToolbarButton("Button2", QImage(), "command2"));
    toolbar.addButton(new ToolbarButton("Button3", QImage(), "command3"));

    BOOST_CHECK( !toolbar.getButtonAt( QPoint( 128, -10 )));
    BOOST_CHECK( !toolbar.getButtonAt( QPoint( 128, 64 )));
    BOOST_CHECK( !toolbar.getButtonAt( QPoint( 512, 10 )));
    BOOST_CHECK( !toolbar.getButtonAt( QPoint( 784, 10 )));

    const ToolbarButton* button1 = toolbar.getButtonAt( QPoint( 128, 10 ));
    BOOST_REQUIRE( button1 );
    BOOST_CHECK_EQUAL( button1->caption.toStdString(), "Button1" );
    BOOST_CHECK_EQUAL( button1->command.toStdString(), "command1" );

    const ToolbarButton* button2 = toolbar.getButtonAt( QPoint( 512/3 + 1, 50 ));
    BOOST_REQUIRE( button2 );
    BOOST_CHECK_EQUAL( button2->caption.toStdString(), "Button2" );
    BOOST_CHECK_EQUAL( button2->command.toStdString(), "command2" );

    const ToolbarButton* button3 = toolbar.getButtonAt( QPoint( 512-1, 50 ));
    BOOST_REQUIRE( button3 );
    BOOST_CHECK_EQUAL( button3->caption.toStdString(), "Button3" );
    BOOST_CHECK_EQUAL( button3->command.toStdString(), "command3" );
}
