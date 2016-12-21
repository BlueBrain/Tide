/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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

#ifndef RESTCONTROLLER_H
#define RESTCONTROLLER_H

#include "control/DisplayGroupController.h"
#include "RestCommand.h"
#include "scene/DisplayGroup.h"

#include "types.h"
#include <memory>
#include <QObject>
#include <zeroeq/http/server.h>

/**
 * Enable remote control of windows through a REST API
 */
class RestController : public QObject
{
    Q_OBJECT

public:
    /**
     * Construct a window controller exposed by a ZeroEQ http server.
     *
     * @param server used to register HTTP endpoints.
     * @param group target for control commands.
     */
    RestController( zeroeq::http::Server& httpServer, DisplayGroup& group );

private:
    DisplayGroup& _group;

    std::unique_ptr<DisplayGroupController> _controller;

    RestCommand _toggleSelectWindow{ "tide/toggleSelectWindow" };
    RestCommand _moveToFront{ "tide/moveWindowToFront" };
    RestCommand _moveWindowtoFullscreen{ "tide/moveWindowToFullscreen" };
    RestCommand _unfocusWindow{ "tide/unfocusWindow" };

    bool _handleToggleSelectWindow( QString id );
    bool _handleDeselectWindows();
    bool _handleExitFullScreen();
    bool _handleFocusWindows();
    bool _handleMoveWindow( const std::string& payload );
    bool _handleMoveWindowToFront( QString id );
    bool _handleMoveWindowToFullscreen( QString id );
    bool _handleResizeWindow( const std::string& payload );
    bool _handleUnfocusWindow( const QString& id );
    bool _handleUnfocusWindows();
};

#endif
