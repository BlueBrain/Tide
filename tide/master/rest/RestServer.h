/*********************************************************************/
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
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
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#ifndef RESTSERVER_H
#define RESTSERVER_H

#include <rockets/http/response.h>
#include <rockets/qt/socketProcessor.h>
#include <rockets/server.h>

#include <set>

/**
 * A non-blocking REST Server for use in a Qt application.
 */
class RestServer : public rockets::Server, public rockets::http::Filter
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
    explicit RestServer(uint16_t port);

    /** Stop the server. */
    ~RestServer();

    /**
     * Block requests for specified method (other than from localhost).
     * Server will return code 403 (FORBIDDEN).
     *
     * @param method the method which is to be blocked
     */
    void block(rockets::http::Method method);

    /**
     * Unblock requests for specified method.
     *
     * @param method the method which is to be unblocked
     */
    void unblock(rockets::http::Method method);

private:
    std::set<rockets::http::Method> _blockedMethods;
    rockets::qt::SocketProcessor _socketProcessor{*this};

    bool _isBlocked(const rockets::http::Method method) const;
    void _init();
    virtual bool _isWhitelisted(const std::string& source) const;

    bool filter(const rockets::http::Request& request) const final;
    rockets::http::Response getResponse(
        const rockets::http::Request& request) const final;
};

#endif
