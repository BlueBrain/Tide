/*********************************************************************/
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
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

#include "ContentLoader.h"

#include "DisplayGroup.h"
#include "ContentWindow.h"
#include "ContentFactory.h"
#include "ContentWindowController.h"
#include "log.h"

ContentLoader::ContentLoader( DisplayGroupPtr displayGroup )
    : _displayGroup( displayGroup )
{
}

bool ContentLoader::load( const QString& filename,
                          const QPointF& windowCenterPosition,
                          const QSizeF& windowSize )
{
    put_flog( LOG_INFO, "opening: '%s'", filename.toLocal8Bit().constData( ));

    if( isAlreadyOpen( filename ))
    {
        put_flog( LOG_INFO, "file already opened: '%s'",
                  filename.toLocal8Bit().constData( ));
        return false;
    }

    ContentPtr content = ContentFactory::getContent( filename );
    if( !content )
    {
        put_flog( LOG_WARN, "ignoring unsupported file: '%s'",
                  filename.toLocal8Bit().constData( ));
        return false;
    }

    ContentWindowPtr contentWindow( new ContentWindow( content ));
    ContentWindowController controller( *contentWindow, *_displayGroup );

    if( windowSize.isValid( ))
        controller.resize( windowSize );
    else
        controller.adjustSize( SIZE_1TO1_FITTING );

    if( windowCenterPosition.isNull( ))
        controller.moveCenterTo( _displayGroup->getCoordinates().center( ));
    else
        controller.moveCenterTo( windowCenterPosition );

    _displayGroup->addContentWindow( contentWindow );

    return true;
}

bool ContentLoader::isAlreadyOpen( const QString& filename ) const
{
    for( const ContentWindowPtr& window : _displayGroup->getContentWindows( ))
    {
        if( window->getContent()->getURI() == filename )
            return true;
    }
    return false;
}
