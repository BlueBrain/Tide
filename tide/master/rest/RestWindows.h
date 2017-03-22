/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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

#ifndef RESTWINDOWS_H
#define RESTWINDOWS_H

#include "RestServer.h"
#include "scene/DisplayGroup.h"

#include <zeroeq/http/request.h>
#include <zeroeq/http/response.h>

#include <future>
#include <QObject>

/**
 * Exposes display group content to ZeroEQ http server.
 */
class RestWindows : public QObject
{
public:
    /**
     * Construct a JSON list of windows.
     *
     * @param displayGroup DisplayGroup to expose.
     */
    RestWindows( const DisplayGroup& displayGroup );

    /**
     * Expose information on specific window to REST Interface.
     *
     * @param request request with the url part including window uuid
     *        and information type.
     * @return future response containing the requested information.
     */
    std::future<zeroeq::http::Response>
    getWindowInfo( const zeroeq::http::Request& request );

    /**
     * Expose information on all windows to REST Interface.
     *
     * @return future response containing a list of all winodws.
     */
    std::future<zeroeq::http::Response>
    getWindowList( const zeroeq::http::Request& ) const;

private:
    const DisplayGroup& _displayGroup;
    QMap<QString, std::string> _thumbnailCache;

    void _cacheThumbnail( ContentWindowPtr contentWindow );
    std::future<zeroeq::http::Response> _getThumbnail( const QString& uuid );
    std::string _getWindowList() const;
};

#endif
