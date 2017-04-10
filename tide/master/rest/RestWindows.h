/*********************************************************************/
/* Copyright (c) 2016-2017, EPFL/Blue Brain Project                  */
/*                          Pawel Podhajski <pawel.podhajski@epfl.ch>*/
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

#include <QFuture>

#include <future>

/**
 * Exposes the windows in a display group in JSON format.
 *
 * This class also maintains a cache of thumbnails for all windows. The
 * thumbnails are generated asynchronously when window are added.
 *
 * Example client usage:
 * GET /api/windows
 * => 200 { "windows": [ {"title": "Title", "uuid": "abcd", ... } ] }
 *
 * GET /api/windows/abcd/thumbnail
 * => 200 data:image/png;base64----IMAGE DATA----
 */
class RestWindows
{
public:
    /**
     * Construct a JSON list of windows exposed by REST interface.
     *
     * @param displayGroup DisplayGroup to expose.
     */
    RestWindows( const DisplayGroup& displayGroup );

    /**
     * Get the detailed list of all windows.
     *
     * @return JSON response containing the list of all winodws.
     */
    std::future<zeroeq::http::Response>
    getWindowList( const zeroeq::http::Request& ) const;

    /**
     * Get information about a specific window (currently thumbnail only).
     *
     * @param request GET request to the url "endpoint/${uuid}/thumbnail".
     * @return base64 encoded image on success, 204 if the thumbnail is not
     *         ready yet.
     */
    std::future<zeroeq::http::Response>
    getWindowInfo( const zeroeq::http::Request& request ) const;

private:
    const DisplayGroup& _displayGroup;
    QMap<QString, QFuture<std::string>> _thumbnailCache;

    void _cacheThumbnail( ContentWindowPtr contentWindow );
    std::future<zeroeq::http::Response>
    _getThumbnail( const QString& uuid ) const;
};

#endif
