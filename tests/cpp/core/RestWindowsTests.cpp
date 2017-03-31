/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
/*                     Pawel Podhajski <pawel.podhajski@epfl.ch>     */
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

#define BOOST_TEST_MODULE RestWindowsTest

#include <boost/test/unit_test.hpp>

#include "rest/RestWindows.h"
#include "rest/json.h"
#include "scene/ContentFactory.h"
#include "scene/DisplayGroup.h"
#include "thumbnail/thumbnail.h"

#include "DummyContent.h"

#include <zeroeq/http/request.h>
#include <zeroeq/http/response.h>

#include <QBuffer>
#include <QByteArray>
#include <QRegExp>
#include <QString>

namespace
{
const QString imageUri{ "wall.png" };
const QSize thumbnailSize{ 512, 512 };
const QSize wallSize{ 1000, 1000 };
const QRegExp _regex{ "\\{|\\}" };

std::string _getThumbnail()
{
    const auto image = thumbnail::create( imageUri, thumbnailSize );
    QByteArray imageArray;
    QBuffer buffer( &imageArray );
    buffer.open( QIODevice::WriteOnly );
    image.save( &buffer,"PNG" );
    buffer.close();
    return "data:image/png;base64," + imageArray.toBase64().toStdString();
}

zeroeq::http::Request _makeThumbnailRequest( const ContentWindow& window )
{
    const auto uuid = window.getID().toString().replace( _regex, "" );

    zeroeq::http::Request request;
    request.path = uuid.toStdString() + "/thumbnail";
    return request;
}
}

BOOST_AUTO_TEST_CASE( testWindowList )
{
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));

    RestWindows restWindows{ *displayGroup };

    ContentPtr content = ContentFactory::getContent( imageUri );
    ContentWindowPtr contentWindow( new ContentWindow( content ));
    contentWindow->setCoordinates( { QPointF{ 64, 79 },
                                     content->getDimensions() } );
    displayGroup->addContentWindow( contentWindow );

    auto response = restWindows.getWindowList( zeroeq::http::Request( )).get();
    BOOST_CHECK_EQUAL( response.code, 200 );
    const auto& type = response.headers[zeroeq::http::Header::CONTENT_TYPE];
    BOOST_CHECK_EQUAL( type, "application/json" );

    const auto object = json::toObject( response.body );
    BOOST_REQUIRE( object.contains( "windows" ));

    const auto windows = object.value( "windows" ).toArray();
    BOOST_REQUIRE_EQUAL( windows.size(), 1 );
    BOOST_REQUIRE( windows[0].isObject( ));

    const auto window = windows[0].toObject();

    BOOST_CHECK_EQUAL( window.value("aspectRatio").toDouble(), 2.0 );
    BOOST_CHECK_EQUAL( window.value("height").toInt(), 128 );
    BOOST_CHECK_EQUAL( window.value("width").toInt(), 256 );
    BOOST_CHECK_EQUAL( window.value("minHeight").toInt(), 300 );
    BOOST_CHECK_EQUAL( window.value("minWidth").toInt(), 600 );
    BOOST_CHECK_EQUAL( window.value("focus").toBool( true ), false );
    BOOST_CHECK_EQUAL( window.value("fullscreen").toBool( true ), false );
    BOOST_CHECK_EQUAL( window.value("selected").toBool( true ), false );
    BOOST_CHECK_EQUAL( window.value("mode").toInt( 99 ), 0 );
    BOOST_CHECK_EQUAL( window.value("title").toString(), imageUri );
    BOOST_CHECK_EQUAL( window.value("uri").toString(), imageUri );
    const auto uuid = QString( "{%1}" ).arg( window.value("uuid").toString( ));
    BOOST_CHECK_EQUAL( uuid, contentWindow->getID().toString( ));
    BOOST_CHECK_EQUAL( window.value("x").toInt(), 64 );
    BOOST_CHECK_EQUAL( window.value("y").toInt(), 79 );
    BOOST_CHECK_EQUAL( window.value("z").toInt( 99 ), 0 );
}

BOOST_AUTO_TEST_CASE( testWindowInfo )
{
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));

    RestWindows windows{ *displayGroup };

    ContentPtr content = ContentFactory::getContent( imageUri );
    ContentWindowPtr window( new ContentWindow( content ));
    displayGroup->addContentWindow( window );

    auto thumbnailRequest = _makeThumbnailRequest( *window );

    // Thumbnail not ready yet
    auto response = windows.getWindowInfo( thumbnailRequest ).get();
    BOOST_CHECK_EQUAL( response.code, 204 );

    // Wait for async thumnbnail generation to finish
    sleep( 2 );
    response = windows.getWindowInfo( thumbnailRequest ).get();
    BOOST_CHECK_EQUAL( response.code, 200 );
    BOOST_CHECK_EQUAL( response.body, _getThumbnail( ));
    BOOST_CHECK_EQUAL( response.headers[zeroeq::http::Header::CONTENT_TYPE],
                       "image/png" );

    displayGroup->removeContentWindow( window );
    response = windows.getWindowInfo( thumbnailRequest ).get();
    BOOST_CHECK_EQUAL( response.code, 404 );

    thumbnailRequest.path = "uuid/notDefinedAction";
    response = windows.getWindowInfo( thumbnailRequest ).get();
    BOOST_CHECK_EQUAL( response.code, 400 );
}
