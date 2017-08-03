/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
/*                     Pawel Podhajski <pawel.podhajski@epfl.ch>     */
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

#ifndef FILERECEIVER_H
#define FILERECEIVER_H

#include "types.h"

#include <zeroeq/http/server.h>

#include <QMap>
#include <QObject>
#include <QString>

/**
 * Receive HTTP file uploads.
 *
 * Example client usage:
 *
 * POST /api/upload
 * { "filename": "cool image.png", "x": 20, "y": 50 }
 * => 200 { "url" : "cool%20image.png" }
 *
 * PUT /api/upload/cool%20image.png
 * --- BINARY DATA ---
 * => 201
 */
class FileReceiver : public QObject
{
    Q_OBJECT

public:
    using Response = zeroeq::http::Response;

    /**
     * Prepare the upload of a file via REST Interface.
     *
     * @param request JSON POST request with fields: { filename, x, y }.
     *        filename is the desired filename, x and y are the desired
     *        coordinates for opening the content.
     * @return JSON response with the url to use for handleUpload() as { url }.
     */
    std::future<Response> prepareUpload(const zeroeq::http::Request& request);

    /**
     * Upload a file via REST Interface.
     *
     * @param request binary PUT request to the url returned by prepareUpload().
     * @return response with appropiate code and status (201 on success).
     */
    std::future<Response> handleUpload(const zeroeq::http::Request& request);

signals:
    /** Open the uploaded file at the given position. */
    void open(QString uri, QPointF position, BoolCallback callback);

private:
    QMap<QString, QPointF> _preparedPaths;
};

#endif
