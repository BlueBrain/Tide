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
}

HtmlContent::HtmlContent( zeroeq::http::Server& server )
{
    server.handle( http::Verb::GET, "/", []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] = "text/html";
        response.payload = _readFile( ":///html/index.html" );
        return http::make_ready_future( response );
    });

    server.handle( http::Verb::GET, "css/bootstrap.css",
    []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] = "text/css";
        response.payload = _readFile( ":///html/bootstrap.min.css" );
        return http::make_ready_future( response );
    });

    server.handle( http::Verb::GET, "js/bootstrap-treeview.min.js",
                   []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] =
                "application/javascript";
        response.payload = _readFile( ":///html/bootstrap-treeview.min.js" );
        return http::make_ready_future( response );
    });

    server.handle( http::Verb::GET, "img/close.svg", []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] = "image/svg+xml";
        response.payload = _readFile( ":///html/img/close.svg" );
        return http::make_ready_future( response );
    });

    server.handle( http::Verb::GET, "img/focus.svg", []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] = "image/svg+xml";
        response.payload = _readFile( ":///html/img/focus.svg" );
        return http::make_ready_future( response );
    });

    server.handle( http::Verb::GET, "img/loading.gif", []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] = "image/gif";
        QFile file(":///html/img/loading.gif");
        file.open( QIODevice::ReadOnly );
        QByteArray data = file.readAll();
        response.payload = data.toStdString();
        return http::make_ready_future( response );
    });

    server.handle( http::Verb::GET, "js/jquery-3.1.1.min.js",
                   []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] =
                "application/javascript";
        response.payload = _readFile( ":///html/jquery-3.1.1.min.js" );
        return http::make_ready_future( response );
    });

    server.handle( http::Verb::GET, "js/notify.min.js",
                   []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] =
                "application/javascript";
        response.payload = _readFile( ":///html/notify.min.js" );
        return http::make_ready_future( response );
    });

    server.handle( http::Verb::GET, "css/jquery-ui.css",
                   []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] = "text/css";
        response.payload = _readFile( ":///html/jquery-ui.css" );
        return http::make_ready_future( response );
    });

    server.handle( http::Verb::GET, "js/jquery-ui.min.js",
                   []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] =
                "application/javascript";
        response.payload = _readFile( ":///html/jquery-ui.min.js" );
        return http::make_ready_future( response );
    });

    server.handle( http::Verb::GET, "img/maximize.svg", []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] = "image/svg+xml";
        response.payload = _readFile( ":///html/img/maximize.svg" );
        return http::make_ready_future( response );
    });

    server.handle( http::Verb::GET, "js/sweetalert.min.js",
                   []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] =
                "application/javascript";
        response.payload = _readFile( ":///html/sweetalert.min.js" );
        return http::make_ready_future( response );
    });

    server.handle( http::Verb::GET, "css/sweetalert.min.css",
                   []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] = "text/css";
        response.payload = _readFile( ":///html/sweetalert.min.css" );
        return http::make_ready_future( response );
    });

    server.handle( http::Verb::GET, "js/tide.js", []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] =
                "application/javascript";
        response.payload = _readFile( ":///html/tide.js" );
        return http::make_ready_future( response );
    });

    server.handle( http::Verb::GET, "css/styles.css", []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] = "text/css";
        response.payload = _readFile( ":///html/styles.css" );
        return http::make_ready_future( response );
    });

    server.handle( http::Verb::GET, "js/tideVars.js", []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] =
                "application/javascript";
        response.payload = _readFile( ":///html/tideVars.js" );
        return http::make_ready_future( response );
    });

    server.handle( http::Verb::GET, "js/underscore-min.js",
                   []( const std::string& )
    {
        http::Response response{ http::Code::OK };
        response.headers[http::Header::CONTENT_TYPE] =
                "application/javascript";
        response.payload = _readFile( ":///html/underscore-min.js" );
        return http::make_ready_future( response );
    });
}
