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

#ifndef THUMBNAILCACHE_H
#define THUMBNAILCACHE_H

#include "scene/DisplayGroup.h"

#include <rockets/http/helpers.h>

#include <QFuture>
#include <QMap>
#include <QUuid>

/**
 * This class maintains a cache of {512x512} thumbnails for a DisplayGroup.
 *
 * The thumbnails are generated asynchronously when windows are added.
 *
 * Example client usage:
 * GET /api/windows
 * => 200 { "windows": [ {"title": "Title", "uuid": "abcd", ... } ] }
 *
 * GET /api/windows/abcd/thumbnail
 * => 200 data:image/png;base64----IMAGE DATA----
 */
class ThumbnailCache
{
public:
    /**
     * Construct a thumbnail cache to expose to the REST interface.
     *
     * @param displayGroup to monitor.
     */
    ThumbnailCache(const DisplayGroup& displayGroup);

    /**
     * Get the thumbnail of a window.
     *
     * @param uuid of the window.
     * @return base64 encoded image on success, 204 if the thumbnail is not
     *         ready yet, 404 if the thumbnail does not exist (anymore).
     */
    std::future<rockets::http::Response> getThumbnail(const QUuid& uuid) const;

private:
    QMap<QUuid, QFuture<std::string>> _thumbnailCache;

    void _cacheThumbnail(ContentWindowPtr contentWindow);
};

#endif
