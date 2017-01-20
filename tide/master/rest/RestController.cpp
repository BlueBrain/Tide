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

#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

namespace
{
QJsonObject _parseJSON( const std::string& data )
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

RestController::RestController( zeroeq::http::Server& server,
                                DisplayGroup& group )
    : _group( group )
    , _controller( new DisplayGroupController( group ))
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

    server.handlePUT( "tide/resizeWindow", [this]( const std::string& received )
    { return _handleResizeWindow( received ); } );

    server.handlePUT( "tide/moveWindow", [this]( const std::string& received )
    { return _handleMoveWindow( received ); } );

    server.handlePUT( "tide/focusWindows", [this]()
    { return _handleFocusWindows(); });

    server.handlePUT( "tide/unfocusWindows", [this]()
    { return _handleUnfocusWindows(); });

    server.handlePUT( "tide/exitFullScreen", [this]()
    { return _handleExitFullScreen(); } );

    server.handlePUT( "tide/deselectWindows", [this]()
    { return _handleDeselectWindows(); } );

    server.handlePUT( "tide/clear", [this]()
    { _group.clear(); return true; } );
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
   _controller->unfocus( QUuid( id ) );
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
    const auto obj = _parseJSON( payload );
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
    const auto obj = _parseJSON( payload );
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

