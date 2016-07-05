/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
/*                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>         */
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

#include "Launcher.h"

#include "tide/core/ContentFactory.h"
#include "tide/core/thumbnail/ThumbnailProvider.h"

#include "tide/master/localstreamer/CommandLineOptions.h"
#include "tide/master/MasterConfiguration.h"

#include <QHostInfo>
#include <QQmlContext>

namespace
{
const std::string deflectHost( "localhost" );
const QString deflectQmlFile( "qrc:/qml/qml/main.qml" );
const QString thumbnailProviderId( "thumbnail" );
}

Launcher::Launcher( int& argc, char* argv[] )
    : QGuiApplication( argc, argv )
{
    const CommandLineOptions options( argc, argv );
    const MasterConfiguration config( options.getConfiguration( ));

    const auto deflectStreamname = options.getStreamname().toStdString();
    _qmlStreamer.reset( new deflect::qt::QmlStreamer( deflectQmlFile,
                                                      deflectHost,
                                                      deflectStreamname ));

    connect( _qmlStreamer.get(), &deflect::qt::QmlStreamer::streamClosed,
             this, &QCoreApplication::quit );

    auto item = _qmlStreamer->getRootItem();

    // General setup
    item->setProperty( "restPort", config.getWebServicePort( ));
    if( options.getWidth( ))
        item->setProperty( "width", options.getWidth( ));
    if( options.getHeight( ))
        item->setProperty( "height", options.getHeight( ));

    // FileBrowser setup
    const auto filters = ContentFactory::getSupportedFilesFilter();
    item->setProperty( "filesFilter", filters );
    item->setProperty( "rootFilesFolder", config.getContentDir( ));
    item->setProperty( "rootSessionsFolder", config.getSessionsDir( ));

    QQmlEngine* engine = _qmlStreamer->getQmlEngine();
    engine->addImageProvider( thumbnailProviderId, new ThumbnailProvider );
    engine->rootContext()->setContextProperty( "fileInfo", &_fileInfoHelper );

    // DemoLauncher setup
    item->setProperty( "demoServiceUrl", config.getDemoServiceUrl( ));
    item->setProperty( "demoServiceImageFolder",
                       config.getDemoServiceImageFolder( ));
    item->setProperty( "demoServiceDeflectHost", QHostInfo::localHostName( ));
}

Launcher::~Launcher()
{
    QQmlEngine* engine = _qmlStreamer->getQmlEngine();
    engine->removeImageProvider( thumbnailProviderId );
}

bool _send( QInputMethodEvent* keyEvent, QQuickItem* rootItem )
{
    // Work around missing key event support in Qt for offscreen windows.
    const QList<QQuickItem*> items =
            rootItem->findChildren<QQuickItem*>( QString(),
                                                 Qt::FindChildrenRecursively );
    for( QQuickItem* item : items )
    {
        if( item->hasFocus( ))
        {
            QGuiApplication::instance()->sendEvent( item, keyEvent );
            if( keyEvent->isAccepted( ))
                return true;
        }
    }
    return false;
}

bool Launcher::event( QEvent* event_ )
{
    QInputMethodEvent* keyEvent = dynamic_cast<QInputMethodEvent*>( event_ );
    if( keyEvent )
        return _send( keyEvent, _qmlStreamer->getRootItem( ));
    return QGuiApplication::event( event_ );
}
