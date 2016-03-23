/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#include "WebbrowserCommandHandler.h"

#include <deflect/Command.h>
#include "localstreamer/PixelStreamerLauncher.h"

#include "ContentWindow.h"
#include "PixelStreamWindowManager.h"

WebbrowserCommandHandler::WebbrowserCommandHandler( PixelStreamWindowManager& windowManager,
                                                    PixelStreamerLauncher& pixelStreamLauncher,
                                                    const QString& defaultURL )
    : _windowManager( windowManager )
    , _defaultURL( defaultURL )
{
    connect( this, SIGNAL( openWebBrowser( QPointF, QSize, QString )),
             &pixelStreamLauncher,
             SLOT( openWebBrowser( QPointF, QSize, QString )));
}

deflect::CommandType WebbrowserCommandHandler::getType() const
{
    return deflect::COMMAND_TYPE_WEBBROWSER;
}

void WebbrowserCommandHandler::handle( const deflect::Command& command,
                                       const QString& senderUri )
{
    QString url = command.getArguments();
    if( url.isEmpty( ))
        url = _defaultURL;

    QPointF position;

    // Center the new content where the dock is
    // TODO: DISCL-230
    ContentWindowPtr parentWindow = _windowManager.getContentWindow(senderUri);
    if( parentWindow )
        position = parentWindow->getCoordinates().center();

    emit openWebBrowser( position, QSize(), url );
}
