/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
/*                     Pawel Podhajski  <pawel.podhajski@epfl.ch>    */
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

#include "Whiteboard.h"

#include "tide/master/localstreamer/CommandLineOptions.h"
#include "tide/master/localstreamer/QmlKeyInjector.h"
#include "tide/master/MasterConfiguration.h"

#include <QQmlContext>

namespace
{
const std::string deflectHost( "localhost" );
const QString deflectQmlFile( "qrc:/qml/qml/whiteboard.qml" );
}

Whiteboard::Whiteboard( int& argc, char* argv[] )
    : QGuiApplication( argc, argv )
{
    const CommandLineOptions options( argc, argv );
    const MasterConfiguration config( options.getConfiguration( ));

    const auto deflectStreamId = options.getStreamId().toStdString();
    _qmlStreamer.reset( new deflect::qt::QmlStreamer( deflectQmlFile,
                                                      deflectHost,
                                                      deflectStreamId ));

    connect( _qmlStreamer.get(), &deflect::qt::QmlStreamer::streamClosed,
             this, &QCoreApplication::quit );

    auto item = _qmlStreamer->getRootItem();
    item->setProperty( "saveURL", config.getWhiteboardSaveFolder() );

    QQmlEngine* engine = _qmlStreamer->getQmlEngine();
    engine->rootContext()->setContextProperty( "fileInfo", &_fileInfoHelper );
}

bool Whiteboard::event( QEvent* event_ )
{
    if( auto inputEvent = dynamic_cast<QInputMethodEvent*>( event_ ))
        return QmlKeyInjector::send( inputEvent, _qmlStreamer->getRootItem( ));
    return QGuiApplication::event( event_ );
}
