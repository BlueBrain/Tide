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

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "MinimalGlobalQtApp.h"

using namespace zeroeq;

namespace
{
bool _isMethodForbidden(const QNetworkReply::NetworkError error)
{
    // QNetworkReply ContentAccessDenied and ContentOperationNotPermittedError
    //  correspond to HTTP error code 403
    if (error == QNetworkReply::ContentAccessDenied ||
        error == QNetworkReply::ContentOperationNotPermittedError)
        return true;
    return false;
}
}

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

std::pair<std::string, QNetworkReply::NetworkError> sendHttpRequest(const QUrl& url,
                                            const http::Method method)
{
    // create custom temporary event loop on stack
    QEventLoop eventLoop;
    // quit the event-loop when the network request finished
    QNetworkAccessManager manager;
    QObject::connect(&manager, &QNetworkAccessManager::finished, &eventLoop,
                     &QEventLoop::quit);

    std::unique_ptr<QNetworkReply> reply;
    switch (method)
    {
    case http::Method::GET:
        reply.reset(manager.get(QNetworkRequest{url}));
        break;
    case http::Method::POST:
        reply.reset(manager.post(QNetworkRequest{url}, ""));
        break;
    case http::Method::PUT:
        reply.reset(manager.put(QNetworkRequest{url}, ""));
        break;
    case http::Method::OPTIONS:
        reply.reset(manager.sendCustomRequest(QNetworkRequest{url}, "OPTIONS"));
        break;
    case http::Method::DELETE:
        reply.reset(manager.deleteResource(QNetworkRequest{url}));
        break;
    case http::Method::PATCH:
        reply.reset(manager.sendCustomRequest(QNetworkRequest{url}, "PATCH"));
        break;
    case http::Method::ALL:
        break;
    }

    eventLoop.exec(); // blocks stack until "finished()" has been called

    const auto response = (reply->error() == QNetworkReply::NoError)
                              ? reply->readAll()
                              : reply->errorString();
    return std::make_pair(response.toStdString(), reply->error());
}

struct TestObject
{
    std::string json = "Hello World!";
};
std::string to_json(const TestObject& obj)
{
    return obj.json;
}

BOOST_AUTO_TEST_CASE(testServerReturnsSimpleContent)
{
    RestServer server;
    BOOST_REQUIRE_GT(server.getPort(), 0);

    TestObject test;
    server.handleGET("test", test);

    const auto url = QString("http://localhost:%1/test").arg(server.getPort());
    const auto response = sendHttpRequest(url, http::Method::GET);

    BOOST_CHECK_EQUAL(response.first, "Hello World!");
    BOOST_CHECK_EQUAL(response.second, int(QNetworkReply::NoError));
}

class MockRestServer : public RestServer
{
public:
    bool skipWhitelistCheck = true;

    // Exposing private respondTo
    std::future<http::Response> mockRespondTo(
        zeroeq::http::Request& request) const
    {
        return RestServer::respondTo(request);
    }

private:
    // Bypass hardcoded localhost exception for block()
    bool _isWhitelisted(const std::string&) const final
    {
        return skipWhitelistCheck;
    }
};

BOOST_AUTO_TEST_CASE(block_all_methods)
{
    MockRestServer server;

    for (int method = 0; method < int(zeroeq::http::Method::ALL); ++method)
    {
        server.handle(zeroeq::http::Method(method), "test",
                      [](const http::Request&) {
                          return http::make_ready_response(http::Code::OK);
                      });
    };

    const auto url = QString("http://localhost:%1/test").arg(server.getPort());

    for (int method = 0; method < int(zeroeq::http::Method::ALL); ++method)
    {
        const auto response =
            sendHttpRequest(url, zeroeq::http::Method(method));
        BOOST_CHECK_EQUAL(response.second, int(QNetworkReply::NoError));
    }

    server.skipWhitelistCheck = false;

    for (int method = 0; method < int(zeroeq::http::Method::ALL); ++method)
    {
        server.block(zeroeq::http::Method(method));
        const auto response =
            sendHttpRequest(url, zeroeq::http::Method(method));
        // localhost is no longer whitelisted because of bypassWhitelist flag.
        BOOST_CHECK(_isMethodForbidden(response.second));
    }
}

BOOST_AUTO_TEST_CASE(test_whitelist)
{
    MockRestServer server;
    http::Request localhostRequest;
    localhostRequest.method = http::Method::PUT;
    localhostRequest.path = "/test";
    localhostRequest.source = "127.0.0.1";
    localhostRequest.body = "das";

    http::Request foreingRequest;
    foreingRequest.method = http::Method::PUT;
    foreingRequest.path = "/test";
    foreingRequest.source = "172.16.0.1";
    foreingRequest.body = "das";

    server.block(http::Method::PUT);

    server.handle(zeroeq::http::Method::PUT, "test", [](const http::Request&) {
        return http::make_ready_response(http::Code::OK);
    });

    server.skipWhitelistCheck = true;

    const auto url = QString("http://localhost:%1/test").arg(server.getPort());

    auto response = server.mockRespondTo(localhostRequest);
    BOOST_CHECK_EQUAL(response.get().code, http::Code::OK);

    server.skipWhitelistCheck = false;

    response = server.mockRespondTo(foreingRequest);
    BOOST_CHECK_EQUAL(response.get().code, http::Code::FORBIDDEN);
}
