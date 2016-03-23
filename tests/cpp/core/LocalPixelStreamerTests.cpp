/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
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

#define BOOST_TEST_MODULE PixelStreamer
#include <boost/test/unit_test.hpp>
namespace ut = boost::unit_test;

#include "localstreamer/CommandLineOptions.h"
#include "localstreamer/PixelStreamerType.h"
#include "localstreamer/PixelStreamerFactory.h"
#include "localstreamer/PixelStreamer.h"
#include "localstreamer/WebkitPixelStreamer.h"
#include "localstreamer/DockPixelStreamer.h"

#include "GlobalQtApp.h"

BOOST_GLOBAL_FIXTURE( GlobalQtApp );

BOOST_AUTO_TEST_CASE( test_local_pixel_streamer_type )
{
    BOOST_CHECK_EQUAL( getStreamerTypeString(PS_UNKNOWN).toStdString(), "unknown" );
    BOOST_CHECK_EQUAL( getStreamerTypeString(PS_WEBKIT).toStdString(), "webkit" );
    BOOST_CHECK_EQUAL( getStreamerTypeString(PS_DOCK).toStdString(), "dock" );

    BOOST_CHECK_EQUAL( getStreamerType(""), PS_UNKNOWN );
    BOOST_CHECK_EQUAL( getStreamerType("zorglump"), PS_UNKNOWN );
    BOOST_CHECK_EQUAL( getStreamerType("webkit"), PS_WEBKIT );
    BOOST_CHECK_EQUAL( getStreamerType("dock"), PS_DOCK );
}

BOOST_AUTO_TEST_CASE( test_local_pixel_streamer_factory_unknown_type )
{
    CommandLineOptions options;
    // Create should return a nullptr
    BOOST_CHECK( !PixelStreamerFactory::create( options ));
}

BOOST_AUTO_TEST_CASE( test_local_pixel_streamer_factory_webkit_type )
{
    if( !hasGLXDisplay( ))
      return;

    CommandLineOptions options;
    options.setPixelStreamerType(PS_WEBKIT);
    PixelStreamer* streamer = PixelStreamerFactory::create( options );

    BOOST_CHECK( streamer );

    WebkitPixelStreamer* webkit = dynamic_cast<WebkitPixelStreamer*>(streamer);
    BOOST_CHECK( webkit );

    delete streamer;
}

BOOST_AUTO_TEST_CASE( test_local_pixel_streamer_factory_dock_type )
{
    if( !hasGLXDisplay( ))
      return;

    CommandLineOptions options;
    options.setPixelStreamerType(PS_DOCK);
    PixelStreamer* streamer = PixelStreamerFactory::create( options );

    BOOST_CHECK( streamer );

    DockPixelStreamer* dock = dynamic_cast<DockPixelStreamer*>(streamer);
    BOOST_CHECK( dock );

    delete streamer;
}

