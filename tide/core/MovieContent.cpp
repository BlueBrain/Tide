/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
/*                     Raphael.Dumusc@epfl.ch                        */
/*                     Daniel.Nachbaur@epfl.ch                       */
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

#include "MovieContent.h"

#include "FFMPEGMovie.h"

#include <QtCore/QFileInfo>

BOOST_CLASS_EXPORT_IMPLEMENT( MovieContent )

IMPLEMENT_SERIALIZE_FOR_XML( MovieContent )

namespace
{
const QString ICON_PAUSE( "qrc:///img/pause.svg" );
const QString ICON_PLAY( "qrc:///img/play.svg" );
}

MovieContent::MovieContent( const QString& uri )
    : Content( uri )
    , _controlState( STATE_LOOP )
{
    _createActions();
}

MovieContent::MovieContent()
    : _controlState( STATE_LOOP )
{
}

CONTENT_TYPE MovieContent::getType() const
{
    return CONTENT_TYPE_MOVIE;
}

bool MovieContent::readMetadata()
{
    QFileInfo file( getURI( ));
    if( !file.exists() || !file.isReadable( ))
        return false;

    const FFMPEGMovie movie( getURI( ));
    if( !movie.isValid( ))
        return false;

    _size = QSize( movie.getWidth(), movie.getHeight( ));
    return true;
}

const QStringList& MovieContent::getSupportedExtensions()
{
    static QStringList extensions;

    if( extensions.empty( ))
    {
        extensions << "mov" << "avi" << "mp4" << "mkv" << "mpg" << "mpeg"
                   << "flv" << "wmv";
    }

    return extensions;
}

ControlState MovieContent::getControlState() const
{
    return _controlState;
}

void MovieContent::play()
{
    _controlState = (ControlState)(_controlState & ~STATE_PAUSED);

    emit modified();
}

void MovieContent::pause()
{
    _controlState = (ControlState)(_controlState | STATE_PAUSED);

    emit modified();
}

void MovieContent::_createActions()
{
    ContentAction* playPauseAction = new ContentAction();
    playPauseAction->setCheckable( true );
    playPauseAction->setIcon( ICON_PAUSE );
    playPauseAction->setIconChecked( ICON_PLAY );
    playPauseAction->setChecked( _controlState & STATE_PAUSED );
    connect( playPauseAction, SIGNAL( checked( )), this, SLOT( pause( )));
    connect( playPauseAction, SIGNAL( unchecked( )), this, SLOT( play( )));
    _actions.add( playPauseAction );
}
