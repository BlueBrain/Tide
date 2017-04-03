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

#include "RestController.h"

#include "control/ContentWindowController.h"
#include "json.h"

#include <zeroeq/http/helpers.h>

using namespace std::placeholders;
using namespace zeroeq;

namespace
{
QString _toWindowId( const std::string& httpWindowId )
{
    return QString( "{%1}" ).arg( httpWindowId.c_str( ));
}
}

RestController::RestController( http::Server& server, DisplayGroup& group )
    : _group( group )
    , _controller( new DisplayGroupController( group ))
{
    server.handlePUT( "tide/deselect-windows", http::PUTFunc{
                      std::bind( &RestController::_deselectWindows, this )});

    server.handlePUT( "tide/exit-fullscreen", http::PUTFunc{
                      std::bind( &RestController::_exitFullScreen, this )});

    server.handlePUT( "tide/focus-windows", http::PUTFunc{
                      std::bind( &RestController::_focusWindows, this )});

    server.handlePUT( "tide/unfocus-windows", http::PUTFunc{
                      std::bind( &RestController::_unfocusWindows, this )});

    server.handlePUT( "tide/move-window-to-front",
                      std::bind( &RestController::_moveWindowToFront, this,
                                 _1 ));

    server.handlePUT( "tide/move-window",
                      std::bind( &RestController::_moveWindow, this, _1 ));

    server.handlePUT( "tide/move-window-to-fullscreen",
                      std::bind( &RestController::_moveWindowToFullscreen, this,
                                 _1 ));

    server.handlePUT( "tide/resize-window",
                      std::bind( &RestController::_resizeWindow, this, _1 ));

    server.handlePUT( "tide/toggle-select-window",
                      std::bind( &RestController::_toggleSelectWindow, this,
                                 _1 ));

    server.handlePUT( "tide/unfocus-window",
                      std::bind( &RestController::_unfocusWindow, this, _1 ));

    server.handle( http::Method::DELETE, "tide/windows/",
                   [this]( const http::Request& request )
    {
        if( auto window = _group.getContentWindow( _toWindowId( request.path )))
        {
            _group.removeContentWindow( window );
            return make_ready_future( http::Response{ http::Code::OK } );
        }
        return make_ready_future( http::Response{ http::Code::NO_CONTENT } );
    });
}

bool RestController::_deselectWindows()
{
    _controller->deselectAll();
    return true;
}

bool RestController::_exitFullScreen()
{
    _controller->exitFullscreen();
    return true;
}

bool RestController::_focusWindows()
{
    _controller->focusSelected();
    return true;
}

bool RestController::_unfocusWindows()
{
    _controller->unfocusAll();
    return true;
}

bool RestController::_moveWindowToFront( const std::string& payload )
{
    const auto id = json::toObject( payload )["uri"].toString();
    return _controller->moveWindowToFront( id );
}

bool RestController::_moveWindowToFullscreen( const std::string& payload )
{
    const auto id = json::toObject( payload )["uri"].toString();
    return _controller->showFullscreen( id );
}

bool RestController::_toggleSelectWindow( const std::string& payload )
{
    const auto id = json::toObject( payload )["uri"].toString();
    if( auto window = _group.getContentWindow( id ))
    {
        window->setSelected( !window->isSelected( ));
        return true;
    }
    return false;
}

bool RestController::_unfocusWindow( const std::string& payload )
{
    const auto id = json::toObject( payload )["uri"].toString();
    return _controller->unfocus( id );
}

bool RestController::_moveWindow( const std::string& payload )
{
    const auto obj = json::toObject( payload );
    if( obj.empty() || !obj["uri"].isString() || !obj["x"].isDouble() ||
        !obj["y"].isDouble( ))
    {
        return false;
    }

    auto window = _group.getContentWindow( obj["uri"].toString( ));
    if( !window )
       return false;

    const auto windowPos = QPointF( obj["x"].toDouble(), obj["y"].toDouble( ));
    const auto fixedPoint = WindowPoint::TOP_LEFT;

    ContentWindowController( *window, _group ).moveTo( windowPos, fixedPoint );

    _controller->moveWindowToFront( window->getContent()->getURI( ));
    return true;
}

bool RestController::_resizeWindow( const std::string& payload )
{
    const auto obj = json::toObject( payload );
    if( obj.empty() || !obj["uri"].isString( ) || !obj["w"].isDouble() ||
        !obj["h"].isDouble() || !obj["centered"].isBool( ))
    {
        return false;
    }

    auto window = _group.getContentWindow( obj["uri"].toString( ));
    if( !window )
       return false;

    const auto windowSize = QSizeF( obj["w"].toDouble(), obj["h"].toDouble( ));
    const auto fixedPoint = obj["centered"].toBool() ? WindowPoint::CENTER :
                                                       WindowPoint::TOP_LEFT;

    ContentWindowController( *window, _group ).resize( windowSize, fixedPoint );
    return true;
}
