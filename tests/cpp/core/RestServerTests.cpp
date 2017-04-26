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

#define BOOST_TEST_MODULE RestServerTests
#include <boost/test/unit_test.hpp>

#include "rest/RestServer.h"

#include <zeroeq/uri.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "MinimalGlobalQtApp.h"
BOOST_GLOBAL_FIXTURE(MinimalGlobalQtApp);

BOOST_AUTO_TEST_CASE(testDefaultPort)
{
    RestServer server;
    BOOST_CHECK_GT(server.getPort(), 0);
}

BOOST_AUTO_TEST_CASE(testUnavailablePort)
{
    // system port (<1024)
    BOOST_CHECK_THROW(RestServer server{80}, std::runtime_error);
}

QString sendHttpRequest(const QUrl& url)
{
    // create custom temporary event loop on stack
    QEventLoop eventLoop;
    // quit the event-loop when the network request finished
    QNetworkAccessManager manager;
    QObject::connect(&manager, &QNetworkAccessManager::finished, &eventLoop,
                     &QEventLoop::quit);

    std::unique_ptr<QNetworkReply> reply(manager.get(QNetworkRequest{url}));
    eventLoop.exec(); // blocks stack until "finished()" has been called

    return (reply->error() == QNetworkReply::NoError) ? reply->readAll()
                                                      : reply->errorString();
}

BOOST_AUTO_TEST_CASE(testServerReturnsSimpleContent)
{
    RestServer server;
    BOOST_REQUIRE_GT(server.getPort(), 0);

    server.get().handleGET("test", [] { return "Hello World!"; });

    const auto url = QString("http://localhost:%1/test").arg(server.getPort());
    const auto response = sendHttpRequest(url);

    BOOST_CHECK_EQUAL(response.toStdString(), "Hello World!");
}
