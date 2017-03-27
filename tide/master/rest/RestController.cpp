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

#include "RestController.h"

#include "control/ContentWindowController.h"
#include "log.h"

#include <zeroeq/http/helpers.h>

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

using namespace zeroeq;

namespace
{
QJsonObject _toJSONObject( const std::string& data )
{
    const auto input = QByteArray::fromRawData( data.c_str(), data.size( ));
    const auto doc = QJsonDocument::fromJson( input );

    if( doc.isNull() || !doc.isObject( ))
    {
        put_flog( LOG_INFO, "Error parsing JSON string: '%s'", data.c_str( ));
        return QJsonObject{};
    }
    return doc.object();
}
}

RestController::RestController( http::Server& server,
                                DisplayGroup& group,
                                const MasterConfiguration& config )
    : _group( group )
    , _controller( new DisplayGroupController( group ))
    , _config ( config )
{
    server.handlePUT(_moveToFront);
    connect ( &_moveToFront, &RestCommand::received,
              this, &RestController::_handleMoveWindowToFront );

    server.handlePUT(_toggleSelectWindow);
    connect ( &_toggleSelectWindow, &RestCommand::received,
              this, &RestController::_handleToggleSelectWindow );

    server.handlePUT(_unfocusWindow);
    connect ( &_unfocusWindow, &RestCommand::received,
              this, &RestController::_handleUnfocusWindow );

    server.handlePUT(_moveWindowtoFullscreen);
    connect ( &_moveWindowtoFullscreen, &RestCommand::received,
              this, &RestController::_handleMoveWindowToFullscreen );

    server.handlePUT( "tide/resize-window", [this]( const std::string& received )
    { return _handleResizeWindow( received ); } );

    server.handlePUT( "tide/move-window", [this]( const std::string& received )
    { return _handleMoveWindow( received ); } );

    server.handlePUT( "tide/focus-windows", [this]()
    { return _handleFocusWindows(); });

    server.handlePUT( "tide/unfocus-windows", [this]()
    { return _handleUnfocusWindows(); });

    server.handlePUT( "tide/exit-fullscreen", [this]()
    { return _handleExitFullScreen(); } );

    server.handlePUT( "tide/deselect-windows", [this]()
    { return _handleDeselectWindows(); } );

    server.handle( http::Method::PUT, "tide/clear",
                  [this]( const zeroeq::http::Request& )
    {
        _group.clear();
        http::Response response;
        response.headers[ http::Header::CONTENT_TYPE ] = "application/json";
        QJsonObject obj;
        obj["status"] = "cleared";
        response.body = QJsonDocument{ obj }.toJson().toStdString();
        return make_ready_future( response );
    });

    server.handle( http::Method::DELETE, "tide/windows",
                   [this]( const zeroeq::http::Request& request )
    {
        const QUuid uuid( QString::fromStdString( request.path ));

        if( auto window = _group.getContentWindow(uuid ))
        {
            _group.removeContentWindow( window );
            return make_ready_future( http::Response{ http::Code::OK } );
        }
        return make_ready_future( http::Response{ http::Code::NO_CONTENT });
    });

    server.handle( http::Method::PUT, "tide/load",
                  [this]( const zeroeq::http::Request& request )
    {
        const auto obj = _toJSONObject( request.body );
        if( obj.empty( ))
            return http::make_ready_future( http::Response{
                                                  http::Code::BAD_REQUEST } );
        const QString uri  = obj["uri"].toString( );
        auto future_response = std::async(std::launch::deferred, [this, uri]()
        {
            auto promise = std::make_shared<std::promise<bool>>();
            auto future = promise->get_future();
            if( uri.isEmpty( ))
            {
                _group.clear();
                return http::Response{ http::Code::OK };
            }
            if( QDir::isRelativePath( uri ))
                emit load( _config.getSessionsDir() + "/" + uri, promise );
            else
                emit load( uri, promise );
            if( !future.get( ))
                return http::Response{ http::Code::INTERNAL_SERVER_ERROR };

            return http::Response{ http::Code::OK };
        });
        return std::move( future_response );
    });

    server.handle( http::Method::PUT, "tide/open",
                  [this]( const zeroeq::http::Request& request )
    {
        const auto obj = _toJSONObject( request.body );
        if( obj.empty( ))
            return http::make_ready_future( http::Response
                                            { http::Code::BAD_REQUEST });

        const QString uri  = obj["uri"].toString( );
        auto future_response = std::async(std::launch::deferred,
                                          [this, uri]()
        {
            auto promise = std::make_shared<std::promise<bool>>();
            auto future = promise->get_future();

            if( QDir::isRelativePath( uri ))
                emit open( _config.getContentDir() + "/" + uri, promise);
            else
                emit open( uri, promise );
            if ( !future.get( ))
                return http::Response{ http::Code::INTERNAL_SERVER_ERROR };
            return http::Response{ http::Code::OK };
        });
        return std::move( future_response );
    });

    server.handle( http::Method::PUT, "tide/save",
                  [this]( const zeroeq::http::Request& request )
    {
        const auto obj = _toJSONObject( request.body );
        if( obj.empty( ))
            return http::make_ready_future( http::Response{
                                                  http::Code::BAD_REQUEST });

        const QString uri  = obj["uri"].toString( );
        auto future_response = std::async(std::launch::deferred,
                                          [this, uri ]()
        {
            auto promise = std::make_shared<std::promise<bool>>();
            auto future = promise->get_future();

            if( QDir::isRelativePath( uri ))
                emit save( _config.getSessionsDir() + "/" + uri, promise );
            else
                emit save( uri, promise );
            if ( !future.get( ))
                return http::Response{ http::Code::INTERNAL_SERVER_ERROR };
            return http::Response{ http::Code::OK };
        });
        return std::move( future_response );
    });
}

bool RestController::_handleMoveWindowToFront( const QString uri )
{
    _controller->moveWindowToFront( uri );
    return true;
}

bool RestController::_handleFocusWindows()
{
    _controller->focusSelected();
    return true;
}
bool RestController::_handleUnfocusWindow( const QString& id )
{
    _controller->unfocus( QUuid( id ));
    return true;
}
bool RestController::_handleUnfocusWindows()
{
    _controller->unfocusAll();
    return true;
}
bool RestController::_handleMoveWindowToFullscreen( const QString uri )
{
    _controller->showFullscreen( uri );
    return true;
}
bool RestController::_handleExitFullScreen()
{
    _controller->exitFullscreen();
    return true;
}

bool RestController::_handleResizeWindow( const std::string& payload )
{
    const auto obj = _toJSONObject( payload );
    if( obj.empty( ))
        return false;
    const auto windowSize = QSizeF( obj["w"].toDouble(), obj["h"].toDouble( ));
    auto window = _group.getContentWindow( obj["uri"].toString( ));
    if( !window )
       return false;
    ContentWindowController controller( *window, _group );
    if( obj["centered"].toInt( ))
        controller.resize( windowSize, WindowPoint::CENTER );
    else
        controller.resize( windowSize, WindowPoint::TOP_LEFT );
    return true;
}

bool RestController::_handleMoveWindow( const std::string& payload )
{
    const auto obj = _toJSONObject( payload );
    if( obj.empty( ))
        return false;

    const QString uri = obj["uri"].toString( );
    auto window = _group.getContentWindow( uri );
    if( !window )
       return false;

    ContentWindowController controller( *window, _group );
    const auto windowPos = QPointF( obj["x"].toDouble(), obj["y"].toDouble( ));
    controller.moveTo( windowPos, WindowPoint::TOP_LEFT );
    _controller->moveWindowToFront( uri );
    return true;
}

bool RestController::_handleToggleSelectWindow( const QString id )
{
    if( auto window = _group.getContentWindow( QUuid( id )))
    {
        window->setSelected( !window->isSelected( ));
        return true;
    }
    return false;
}

bool RestController::_handleDeselectWindows()
{
    _controller->deselectAll();
    return true;
}

