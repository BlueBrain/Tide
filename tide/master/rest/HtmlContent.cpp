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

#include "HtmlContent.h"

#include <zeroeq/http/helpers.h>
#include <zeroeq/http/response.h>

#include <QFile>
#include <QTextStream>

using namespace zeroeq;

namespace
{
std::string _readFile( const QString& uri )
{
    QFile file( uri );
    file.open( QIODevice::ReadOnly | QIODevice::Text );
    QTextStream in( &file );
    return in.readAll().toStdString();
}

std::future<zeroeq::http::Response> _makeResponse( const std::string& type,
                                                   const QString & file )
{
    http::Response response{ http::Code::OK };
    response.headers[http::Header::CONTENT_TYPE] = type;
    response.body = _readFile( file );
    return http::make_ready_future( response );
}

}
HtmlContent::HtmlContent( zeroeq::http::Server& server )
{
    server.handle( http::Method::GET, "/", []( const zeroeq::http::Request&  )
    {
        return _makeResponse( "text/html", ":///html/index.html" );
    });

    server.handle( http::Method::GET, "css/bootstrap.css",
                   []( const zeroeq::http::Request& )
    {
        return _makeResponse( "text/css", ":///html/bootstrap.min.css" );
    });

    server.handle( http::Method::GET, "js/bootstrap-treeview.min.js",
                   []( const zeroeq::http::Request& )
    {
        return _makeResponse( "application/javascript",
                              ":///html/bootstrap-treeview.min.js" );
    });

    server.handle( http::Method::GET, "img/close.svg",
                   []( const zeroeq::http::Request& )
    {
        return _makeResponse( "image/svg+xml", ":///html/img/close.svg" );
    });

    server.handle( http::Method::GET, "img/focus.svg",
                   []( const zeroeq::http::Request& )
    {
        return _makeResponse( "image/svg+xml", ":///html/img/focus.svg" );
    });

    server.handle( http::Method::GET, "img/loading.gif",
                   []( const zeroeq::http::Request& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] = "image/gif";
        QFile file(":///html/img/loading.gif");
        file.open( QIODevice::ReadOnly );
        response.body = file.readAll().toStdString();
        return http::make_ready_future( response );
    });

    server.handle( http::Method::GET, "js/jquery-3.1.1.min.js",
                   []( const zeroeq::http::Request& )
    {
        return _makeResponse( "application/javascript",
                              ":///html/jquery-3.1.1.min.js" );
    });

    server.handle( http::Method::GET, "js/notify.min.js",
                   []( const zeroeq::http::Request& )
    {
        return _makeResponse( "application/javascript",
                              ":///html/notify.min.js" );
    });

    server.handle( http::Method::GET, "css/jquery-ui.css",
                   []( const zeroeq::http::Request& )
    {
        return _makeResponse( "text/css", ":///html/jquery-ui.css" );
    });

    server.handle( http::Method::GET, "js/jquery-ui.min.js",
                   []( const zeroeq::http::Request& )
    {
        return _makeResponse( "application/javascript",
                              ":///html/jquery-ui.min.js" );
    });

    server.handle( http::Method::GET, "img/maximize.svg", 
                   []( const zeroeq::http::Request& )
    {
        return _makeResponse( "image/svg+xml", ":///html/img/maximize.svg" );
    });

    server.handle( http::Method::GET, "js/sweetalert.min.js",
                   []( const zeroeq::http::Request& )
    {
        return _makeResponse( "application/javascript",
                              ":///html/sweetalert.min.js" );
    });

    server.handle( http::Method::GET, "css/sweetalert.min.css",
                   []( const zeroeq::http::Request& )
    {
        return _makeResponse( "text/css", ":///html/sweetalert.min.css" );
    });

    server.handle( http::Method::GET, "js/tide.js",
                   []( const zeroeq::http::Request& )
    {
        return _makeResponse( "application/javascript", ":///html/tide.js" );
    });

    server.handle( http::Method::GET, "css/styles.css",
                   []( const zeroeq::http::Request& )
    {
        return _makeResponse( "text/css", ":///html/styles.css" );
    });

    server.handle( http::Method::GET, "js/tideVars.js",
                   []( const zeroeq::http::Request& )
    {
        return _makeResponse( "application/javascript",
                              ":///html/tideVars.js" );
    });

    server.handle( http::Method::GET, "js/underscore-min.js",
                   []( const zeroeq::http::Request& )
    {
        return _makeResponse( "application/javascript",
                              ":///html/underscore-min.js" );
    });
}
