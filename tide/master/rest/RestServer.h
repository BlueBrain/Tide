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

#ifndef RESTSERVER_H
#define RESTSERVER_H

#include <zeroeq/http/helpers.h>
#include <zeroeq/http/response.h>
#include <zeroeq/http/server.h>

#include <QSocketNotifier>
#include <set>

/**
 * A non-blocking REST Server based on ZeroEQ for use in a Qt application.
 */
class RestServer : public zeroeq::http::Server
{
public:
    /**
     * Start a server with an OS-chosen port.
     * @throw std::runtime_error if a connection issue occured.
     */
    RestServer();

    /**
     * Start a server on a defined port.
     * @param port the TCP port to listen to.
     * @throw std::runtime_error if the port is already in use or a connection
     *        issue occured.
     */
    explicit RestServer(int port);

    /** Stop the server. */
    ~RestServer() = default;

    /** @return the port of the server. */
    int getPort() const;

    /**
     * Expose a JSON-serializable object on an HTTP GET endpoint.
     *
     * @param endpoint for accessing the object.
     * @param object to expose.
     */
    template <typename Obj>
    bool handleGET(const std::string& endpoint, const Obj& object)
    {
        using namespace zeroeq::http;
        return handle(Method::GET, endpoint, [&object](const Request&) {
            return make_ready_response(Code::OK, to_json(object),
                                       "application/json");
        });
    }

    /**
     * Subscribe a JSON-deserializable object on an HTTP PUT endpoint.
     *
     * @param endpoint for modifying the object.
     * @param object to subscribe.
     */
    template <typename Obj>
    bool handlePUT(const std::string& endpoint, Obj& object)
    {
        using namespace zeroeq::http;
        return handle(Method::PUT, endpoint, [&object](const Request& req) {
            const auto success = from_json(object, req.body);
            return make_ready_response(success ? Code::OK : Code::BAD_REQUEST);
        });
    }

    /**
     * Block requests for specified method (other than from localhost).
     * Server will return code 403 (FORBIDDEN).
     *
     * @param method the method which is to be blocked
     */
    void block(zeroeq::http::Method method);

    /**
     * Unblock requests for specified method.
     *
     * @param method the method which is to be unblocked
     */
    void unblock(zeroeq::http::Method method);

protected:
    std::future<zeroeq::http::Response> respondTo(
        zeroeq::http::Request& request) const final;

private:
    QSocketNotifier _socketNotifier{getSocketDescriptor(),
                                    QSocketNotifier::Read};

    std::set<zeroeq::http::Method> _blockedMethods;

    void _init();
    virtual bool _isWhitelisted(const std::string& source) const;
};

#endif
