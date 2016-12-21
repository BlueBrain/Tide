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

#include <QBuffer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace
{
const QRegExp _regex = QRegExp("\\{|\\}");
const QSize thumbnailSize{ 768, 768 };
}

RestWindows::RestWindows( zeroeq::http::Server& server,
                          const DisplayGroup& displayGroup )
    : _httpServer( server ), _displayGroup( displayGroup )
{
    connect( &displayGroup, &DisplayGroup::contentWindowAdded,
              this, &RestWindows::_registerThumbnailUrl );
    connect( &displayGroup, &DisplayGroup::contentWindowRemoved,
              this, &RestWindows::_deregisterThumbnailUrl );
}

std::string RestWindows::getTypeName() const
{
    return "tide/windows";
}

std::string RestWindows::_toJSON() const
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
       window["uuid"] = contentWindow->getID().toString().replace( _regex,"" );
       arr.append(window);
    }
    QJsonObject obj;
    obj["windows"] = arr;
    return QJsonDocument{ obj }.toJson().toStdString();
}

QString _getThumbnailUri( const ContentWindow& window )
{
    const QString id = window.getID().toString().replace( _regex, "" );
    return "tide/windows/" + id + "/thumbnail";
}

void RestWindows::_registerThumbnailUrl( ContentWindowPtr window )
{
    const auto url = _getThumbnailUri( *window );
    const QString uri = window->getContent().get()->getURI();
    _httpServer.handleGET(url.toStdString(), [uri] ()
    {
        const auto image = thumbnail::create( uri, thumbnailSize );
        QByteArray imageArray;
        QBuffer buffer( &imageArray );
        buffer.open( QIODevice::WriteOnly );
        image.save( &buffer, "PNG" );
        return imageArray.toBase64().toStdString();
    });
}

void RestWindows::_deregisterThumbnailUrl( ContentWindowPtr window )
{
    const auto url = _getThumbnailUri( *window );
    _httpServer.remove( url.toStdString( ));
}
