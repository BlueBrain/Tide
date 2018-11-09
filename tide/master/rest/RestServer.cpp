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

#include "RestServer.h"

using namespace rockets;

namespace
{
std::string _getHostname(const std::string& source)
{
    if (source.find(":") != std::string::npos)
        return source.substr(0, source.find(":"));
    else
        return source;
}
}

RestServer::RestServer()
{
    _init();
}

RestServer::RestServer(const uint16_t port)
    : rockets::Server{QString(":%1").arg(port).toStdString(), "", 0}
{
    _init();
}

RestServer::~RestServer()
{
    setSocketListener(nullptr);
}

void RestServer::block(const http::Method method)
{
    _blockedMethods.insert(method);
}

void RestServer::unblock(const http::Method method)
{
    _blockedMethods.erase(method);
}

bool RestServer::_isBlocked(const http::Method method) const
{
    return _blockedMethods.count(method);
}

void RestServer::_init()
{
    setHttpFilter(this);
    setSocketListener(&_socketProcessor);
}

bool RestServer::_isWhitelisted(const std::string& source) const
{
    return _getHostname(source) == "127.0.0.1";
}

bool RestServer::filter(const http::Request& request) const
{
    return _isBlocked(request.method) && !_isWhitelisted(request.origin);
}

http::Response RestServer::getResponse(const http::Request&) const
{
    return http::Response(http::Code::FORBIDDEN);
}
