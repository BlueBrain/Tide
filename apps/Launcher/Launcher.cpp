/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
/*                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>    */
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
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#include "Launcher.h"

#include "FolderModel.h"
#include "ModelFilter.h"

#include "tide/core/scene/ContentFactory.h"
#include "tide/core/thumbnail/ThumbnailProvider.h"

#include "tide/master/localstreamer/CommandLineOptions.h"
#include "tide/master/localstreamer/QmlKeyInjector.h"

#include <tide/core/version.h>

#include <QDirModel>
#include <QHostInfo>
#include <QQmlContext>

namespace
{
const std::string deflectHost("localhost");
const QString deflectQmlFile("qrc:/launcher/qml/main.qml");
const QString thumbnailProviderId("thumbnail");

QQuickImageProvider* _makeImageProvider()
{
#if TIDE_ASYNC_THUMBNAIL_PROVIDER
    return new AsyncThumbnailProvider();
#else
    return new ThumbnailProvider();
#endif
}
} // namespace

Launcher::Launcher(int& argc, char* argv[])
    : QApplication(argc, argv)
{
    qmlRegisterType<FolderModel>("Launcher", 1, 0, "FolderModel");
    qmlRegisterType<ModelFilter>("Launcher", 1, 0, "ModelFilter");

    const CommandLineOptions options(argc, argv);

    const auto deflectStreamId = options.streamId.toStdString();
    _qmlStreamer.reset(new deflect::qt::QmlStreamer(deflectQmlFile, deflectHost,
                                                    deflectStreamId));
    _qmlStreamer->setRenderInterval(30);
    connect(_qmlStreamer.get(), &deflect::qt::QmlStreamer::streamClosed, this,
            &QCoreApplication::quit);

    auto item = _qmlStreamer->getRootItem();

    // General setup
    item->setProperty("tideVersion", TIDE_VERSION_STRING);
    item->setProperty("tideRevision",
                      QString::number(TIDE_VERSION_REVISION, 16));
    item->setProperty("restPort", static_cast<int>(options.webservicePort));
    item->setProperty("powerButtonVisible", options.showPowerButton);
    if (options.width)
        item->setProperty("width", options.width);
    if (options.height)
        item->setProperty("height", options.height);

    // FileBrowser setup
    item->setProperty("filesFilter", ContentFactory::getSupportedFilesFilter());
    item->setProperty("rootFilesFolder", options.contentsDir);
    item->setProperty("rootSessionsFolder", options.sessionsDir);

    auto engine = _qmlStreamer->getQmlEngine();
    engine->rootContext()->setContextProperty("fileInfo", &_fileInfoHelper);
    engine->addImageProvider(thumbnailProviderId, _makeImageProvider());

    // DemoLauncher setup
    item->setProperty("demoServiceUrl", options.demoServiceUrl);
    item->setProperty("demoServiceDeflectHost", QHostInfo::localHostName());
}

bool Launcher::event(QEvent* event_)
{
    if (auto inputEvent = dynamic_cast<QInputMethodEvent*>(event_))
        return QmlKeyInjector::send(inputEvent, _qmlStreamer->getRootItem());
    return QGuiApplication::event(event_);
}
