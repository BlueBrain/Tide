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

#define BOOST_TEST_MODULE RESTCommand
#include <boost/test/unit_test.hpp>

#include "rest/RestCommand.h"
#include "rest/StaticContent.h"

BOOST_AUTO_TEST_CASE( test_receive_json_command )
{
    RestCommand command( "tide::open" );

    QString receivedUri;
    QObject::connect( &command, &RestCommand::received, [&]( const QString uri )
    {
        receivedUri = uri;
    });

    BOOST_CHECK_EQUAL( command.getTypeName(), "tide::open" );

    BOOST_CHECK( !command.fromJSON( "I am not a json string" ));
    BOOST_REQUIRE( receivedUri.isEmpty( ));

    BOOST_CHECK( !command.fromJSON( "{\"invalid_key\":\"image.png\"}" ));
    BOOST_REQUIRE( receivedUri.isEmpty( ));

    BOOST_CHECK( command.fromJSON( "{\"uri\":\"image.png\"}" ));
    BOOST_CHECK_EQUAL( receivedUri.toStdString(), "image.png" );
}

BOOST_AUTO_TEST_CASE( test_static_content )
{
    const StaticContent content( "tide::info", "Welcome to Tide!" );

    BOOST_CHECK_EQUAL( content.getTypeName(), "tide::info" );
    BOOST_CHECK_EQUAL( content.toJSON(), "Welcome to Tide!" );
}
