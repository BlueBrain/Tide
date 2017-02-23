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

#include "RestWindows.h"

#include "control/ContentWindowController.h"
#include "scene/ContentWindow.h"
#include "thumbnail/thumbnail.h"

#include "log.h"

#include <QBuffer>
#include <QtConcurrent>
#include <QFuture>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QThreadPool>

using namespace zeroeq;

namespace
{
const QRegExp _regex = QRegExp("\\{|\\}");
const QSize thumbnailSize{ 512, 512 };

QString _getUuid( const ContentWindow& window )
{
   return window.getID().toString().replace( _regex, "" );
}
}

RestWindows::RestWindows( const DisplayGroup& displayGroup )
    : _displayGroup( displayGroup )
{
    connect( &displayGroup, &DisplayGroup::contentWindowAdded,
             this, &RestWindows::_cacheThumbnail );

    connect( &displayGroup, &DisplayGroup::contentWindowRemoved,
             [this]( ContentWindowPtr window )
    {
        const auto fileName = window->getContent()->getURI();
        if( window->getContent()->getURI().startsWith(QDir::tempPath() + "/" ))
        {
            QDir().remove( fileName );
            put_flog( LOG_INFO, "Removing file: %s",
                      fileName.toLocal8Bit().constData( ));
        }
        _thumbnailCache.remove( _getUuid( *window ));
    });
}

std::future<http::Response> RestWindows::getWindowInfo( const std::string& path,
        const std::string& )
{
    if( path.empty( ))
    {
        http::Response response;
        response.payload = _getWindowList();
        return make_ready_future( response );
    }

    const auto endpoint = QString::fromStdString( path );
    if( endpoint.endsWith( "/thumbnail" ))
    {
        const auto uuid = endpoint.split( "/" )[0];
        return _getThumbnail( uuid );
    }

    return make_ready_future( http::Response{ http::Code::BAD_REQUEST } );
}

std::string RestWindows::_getWindowList() const
{
    const auto& windows = _displayGroup.getContentWindows();

    QJsonArray arr;
    for( const auto& contentWindow : windows )
    {
        if (contentWindow->getContent()->getURI() == "Launcher")
            continue;

	QJsonObject window;
        ContentWindowController controller( *contentWindow , _displayGroup );
        window["aspectRatio"] = contentWindow->getContent()->getAspectRatio();
        window["minWidth"] = controller.getMinSizeAspectRatioCorrect().width();
        window["minHeight"] = controller.getMinSizeAspectRatioCorrect().height();
        window["width"] = contentWindow->getDisplayCoordinates().width();
        window["height"] = contentWindow->getDisplayCoordinates().height();
        window["x"] = contentWindow->getDisplayCoordinates().x();
        window["y"] = contentWindow->getDisplayCoordinates().y();
        window["z"] = _displayGroup.getZindex( contentWindow );
        window["title"] = contentWindow->getContent()->getTitle();
        window["mode"] = contentWindow->getMode();
        window["selected"] = contentWindow->isSelected();
        window["fullscreen"] = contentWindow->isFullscreen();
        window["focus"] = contentWindow->isFocused();
        window["uri"] = contentWindow->getContent()->getURI();
        window["uuid"] = _getUuid( *contentWindow );
        arr.append(window);
    }
    QJsonObject obj;
    obj["windows"] = arr;
    return QJsonDocument{ obj }.toJson().toStdString();
}

void RestWindows::_cacheThumbnail( ContentWindowPtr window )
{
    QtConcurrent::run( QThreadPool::globalInstance(),
                       [this, window]()
    {
        const QString& uri = window->getContent().get()->getURI();
        const auto uuid = _getUuid( *window );

        const auto image = thumbnail::create( uri, thumbnailSize );
        QByteArray imageArray;
        QBuffer buffer( &imageArray );
        buffer.open( QIODevice::WriteOnly );
        if( !image.save( &buffer, "PNG" ))
        {
            _thumbnailCache[uuid] = std::string();
            return;
        }
        buffer.close();
        _thumbnailCache[uuid] = "data:image/png;base64," +
                                imageArray.toBase64().toStdString();
    });
}

std::future<zeroeq::http::Response>
RestWindows::_getThumbnail( const QString& uuid )
{
    if( !_thumbnailCache.contains( uuid ))
        return make_ready_future( http::Response{ http::Code::NO_CONTENT } );

    const auto& image = _thumbnailCache[ uuid ];
    if( image.empty( ))
        return make_ready_future( http::Response{ http::Code::NOT_FOUND } );

    http::Response response{ http::Code::OK };
    response.headers[http::Header::CONTENT_TYPE] = "image/png";
    response.payload = image;
    return make_ready_future( std::move( response ));
}
