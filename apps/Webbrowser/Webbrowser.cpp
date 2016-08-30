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

#include "Webbrowser.h"

#include "tide/master/localstreamer/CommandLineOptions.h"
#include "tide/master/localstreamer/QmlKeyInjector.h"
#include "WebbrowserContent.h"
#include "WebbrowserHistory.h"

#include <QQmlProperty>

namespace
{
const std::string deflectHost( "localhost" );
const QString deflectQmlFile( "qrc:/qml/qml/Webengine.qml" );
const QString webengineviewItemName( "webengineview" );
const QString searchUrl( "https://www.google.com/search?q=%1" );
}

Webbrowser::Webbrowser( int& argc, char* argv[] )
    : QGuiApplication( argc, argv )
{
    const CommandLineOptions options( argc, argv );

    const auto deflectStreamname = options.getStreamname().toStdString();
    _qmlStreamer.reset( new deflect::qt::QmlStreamer( deflectQmlFile,
                                                      deflectHost,
                                                      deflectStreamname ));

    connect( _qmlStreamer.get(), &deflect::qt::QmlStreamer::streamClosed,
             this, &QCoreApplication::quit );

    auto item = _qmlStreamer->getRootItem();

    // General setup
    if( options.getWidth( ))
        item->setProperty( "width", options.getWidth( ));
    if( options.getHeight( ))
        item->setProperty( "height", options.getHeight( ));

    connect( item, SIGNAL( addressBarTextEntered( QString )),
             this, SLOT( _processAddressBarInput( QString )));

    // Keep pointer to the webengine to send key events
    _webengine = item->findChild<QQuickItem*>( webengineviewItemName );
    _webengine->setProperty( "url", options.getUrl( ));

    connect( _webengine, SIGNAL( urlChanged( )), this, SLOT( _sendData( )));
}

Webbrowser::~Webbrowser() {}

bool Webbrowser::event( QEvent* event_ )
{
    // Work around missing key event support in Qt for offscreen windows
    if( auto inputEvent = dynamic_cast<QInputMethodEvent*>( event_ ))
    {
        if( _webengine->hasFocus( ))
            return QmlKeyInjector::sendToWebengine( inputEvent, _webengine );
        return QmlKeyInjector::send( inputEvent, _qmlStreamer->getRootItem( ));
    }
    return QGuiApplication::event( event_ );
}

bool _canBeHttpUrl( const QString& url )
{
    return url.startsWith( "www." ) ||
            (url.contains( '.' ) && !url.contains( ' ' ));
}

void Webbrowser::_processAddressBarInput( const QString url )
{
    auto address = QUrl{ url };
    if( address.scheme().isEmpty( ))
    {
        if( _canBeHttpUrl( url ))
            address.setScheme( "http" );
        else
            address = QUrl{ searchUrl.arg( url.toHtmlEscaped( )) };
    }
    QQmlProperty::write( _webengine, "url", address );
}

void Webbrowser::_sendData()
{
    // WebEngineHistory is only available from QtWebEngine 1.1 (Qt >= 5.5)
    // Make a "fake" history with the available information
    const auto url = QQmlProperty::read( _webengine, "url" ).toString();
    const auto prev = QQmlProperty::read( _webengine, "canGoBack" ).toBool();
    const auto next = QQmlProperty::read( _webengine, "canGoForward" ).toBool();

    auto urls = std::vector<QString>();
    if( prev )
        urls.push_back( QString( ));
    urls.push_back( url );
    if( next )
        urls.push_back( QString( ));
    const auto history = WebbrowserHistory{ std::move( urls ), prev ? 1 : 0 };

    const auto data = WebbrowserContent::serializeData( history, 0 );
    if( !_qmlStreamer->sendData( data ))
            QGuiApplication::quit();
}
