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

#include "FileCommandHandler.h"

#include "DisplayGroup.h"
#include "ContentLoader.h"
#include "ContentFactory.h"
#include "ContentWindow.h"
#include "StateSerializationHelper.h"
#include "PixelStreamWindowManager.h"
#include "log.h"

#include <deflect/Command.h>

#include <QFileInfo>

FileCommandHandler::FileCommandHandler( DisplayGroupPtr displayGroup,
                                        PixelStreamWindowManager& windowManager)
    : displayGroup_( displayGroup )
    , pixelStreamWindowManager_( windowManager )
{
}

deflect::CommandType FileCommandHandler::getType() const
{
    return deflect::COMMAND_TYPE_FILE;
}

void FileCommandHandler::handle( const deflect::Command& command,
                                 const QString& senderUri )
{
    const QString& uri = command.getArguments();
    const QString& extension = QFileInfo( uri ).suffix().toLower();

    if( extension == "dcx" )
    {
        StateSerializationHelper( displayGroup_ ).load( uri );
    }
    else if( ContentFactory::getSupportedExtensions().contains( extension ))
    {
        ContentLoader loader( displayGroup_ );

        // Center the new content where the dock is
        // TODO: DISCL-230
        QPointF position;
        ContentWindowPtr parentWindow =
                pixelStreamWindowManager_.getContentWindow( senderUri );
        if( parentWindow )
            position = parentWindow->getCoordinates().center();
        loader.load( uri, position );
    }
    else
    {
        put_flog( LOG_WARN, "Received uri with unsupported extension: '%s'",
                  uri.toStdString().c_str( ));
    }
}
