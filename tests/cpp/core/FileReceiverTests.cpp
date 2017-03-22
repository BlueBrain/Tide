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

#define BOOST_TEST_MODULE FileReceiverTests

#include <boost/test/unit_test.hpp>

#include "rest/FileReceiver.h"

#include "scene/DisplayGroup.h"

#include <zeroeq/http/response.h>

#include <QBuffer>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QObject>
#include <QString>

namespace
{
const QString imageUri("wall.png");
const QSize wallSize( 1000, 1000 );
}

BOOST_AUTO_TEST_CASE( testFileReceiver )
{
    DisplayGroupPtr displayGroup( new DisplayGroup( wallSize ));
    FileReceiver fileReceiver;

    bool open = false;
    QObject::connect( &fileReceiver, &FileReceiver::open,
             [&open]( const QString& uri, promisePtr promise )
    {
        Q_UNUSED( uri );
        promise->set_value(true);
        open = true;
    });

    QJsonObject unsupportedFile;
    unsupportedFile["fileName"] = "wall.pn";
    QJsonDocument doc( unsupportedFile );
    auto future = fileReceiver.prepareUpload( doc.toJson().toStdString( ));

    auto response = future.get();
    BOOST_CHECK_EQUAL( response.code, 405 );

    QJsonObject supportedFile;
    supportedFile["fileName"] = "wall.png";
    QJsonDocument doc2( supportedFile );
    future = fileReceiver.prepareUpload( doc2.toJson().toStdString( ));

    response = future.get();
    BOOST_CHECK_EQUAL( response.code, 200 );

    QFile file( imageUri );
    file.open( QIODevice::ReadOnly );
    auto payload = file.readAll().toStdString();

    BOOST_CHECK_EQUAL( open, false );
    future = fileReceiver.handleUpload( "wall.png", payload );
    response = future.get();
    BOOST_CHECK_EQUAL( open, true );
    BOOST_CHECK_EQUAL( response.code, 201);

    future = fileReceiver.handleUpload( "wall.png", payload );
    response = future.get();
    BOOST_CHECK_EQUAL( response.code, 403 );
}
