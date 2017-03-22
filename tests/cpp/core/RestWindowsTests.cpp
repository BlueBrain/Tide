/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
/*                     Pawel Podhajski <pawel.podhajski@epfl.ch>     */
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
#include "scene/ContentFactory.h"
#include "scene/DisplayGroup.h"

#include "DummyContent.h"
#include "thumbnail/thumbnail.h"

#include <zeroeq/http/response.h>

#include <QString>
#include <QByteArray>
#include <QBuffer>
#include <QRegExp>

namespace
{
const QString imageUri("wall.png");
const QSize thumbnailSize{ 512, 512 };
const QSize wallSize( 1000, 1000 );
const QRegExp _regex = QRegExp("\\{|\\}");

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
}

BOOST_AUTO_TEST_CASE( testWindowInfo )
{
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));

    RestWindows windowsContent{ *displayGroup };

    ContentPtr content = ContentFactory::getContent( imageUri );
    ContentWindowPtr window( new ContentWindow( content ));
    displayGroup->addContentWindow( window );

    auto future = windowsContent.getWindowInfo( std::string(), std::string( ));
    auto response = future.get();
    BOOST_CHECK_EQUAL( response.code, 200 );

    const auto uuid = window->getID().toString().replace( _regex, "" );

    future = windowsContent.getWindowInfo( uuid.toStdString() + "/thumbnail",
                                           std::string( ));
    response = future.get();
    BOOST_CHECK_EQUAL( response.code, 204 );

    // Wait for the async thumnbnail generation
    sleep(2);
    future = windowsContent.getWindowInfo( uuid.toStdString() + "/thumbnail",
                                           std::string( ));
    response = future.get();
    BOOST_CHECK_EQUAL( response.payload,_getThumbnail( ));
    BOOST_CHECK_EQUAL( response.code, 200 );
    BOOST_CHECK_EQUAL( response.headers[zeroeq::http::Header::CONTENT_TYPE],
                       "image/png" );

    displayGroup->removeContentWindow( window );

    future = windowsContent.getWindowInfo( uuid.toStdString() + "/thumbnail",
                                           std::string( ));
    response = future.get();
    BOOST_CHECK_EQUAL( response.code, 204 );

    future = windowsContent.getWindowInfo( "uuid/notExists", std::string( ));
    response = future.get();
    BOOST_CHECK_EQUAL( response.code, 400 );

}
