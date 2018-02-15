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

#include "tide/core/scene/WebbrowserContent.h"
#include "tide/core/scene/WebbrowserHistory.h"

#include "tide/master/localstreamer/CommandLineOptions.h"
#include "tide/master/localstreamer/HtmlSelectReplacer.h"
#include "tide/master/localstreamer/QmlKeyInjector.h"

#include <QQmlContext>
#include <QQmlProperty>

namespace
{
const std::string deflectHost("localhost");
const QString deflectQmlFile("qrc:/qml/qml/Webengine.qml");
const QString webengineviewItemName("webengineview");
const QString searchUrl("https://www.google.com/search?q=%1");

int _getLogLevel(const int jsLevel)
{
    switch (jsLevel)
    {
    case 0:
        return LOG_INFO;
    case 1:
        return LOG_WARN;
    case 2:
        return LOG_ERROR;
    default:
        throw std::invalid_argument("unknown log level");
    }
}
}

Webbrowser::Webbrowser(int& argc, char* argv[])
    : QGuiApplication(argc, argv)
    , _selectReplacer(new HtmlSelectReplacer)
{
    const CommandLineOptions options(argc, argv);

    const auto deflectStreamId = options.streamId.toStdString();
    _qmlStreamer.reset(new deflect::qt::QmlStreamer(deflectQmlFile, deflectHost,
                                                    deflectStreamId));

    connect(_qmlStreamer.get(), &deflect::qt::QmlStreamer::streamClosed, this,
            &QCoreApplication::quit);

    auto context = _qmlStreamer->getQmlEngine()->rootContext();
    context->setContextProperty("htmlselect", _selectReplacer.get());

    auto item = _qmlStreamer->getRootItem();

    // General setup
    if (options.width)
        item->setProperty("width", options.width);
    if (options.height)
        item->setProperty("height", options.height);

    connect(item, SIGNAL(addressBarTextEntered(QString)), this,
            SLOT(_processAddressBarInput(QString)));

    // Keep pointer to the webengine to send key events
    _webengine = item->findChild<QQuickItem*>(webengineviewItemName);
    _webengine->setProperty("url", options.url);

    connect(_webengine, SIGNAL(urlChanged()), this, SLOT(_sendData()));
    connect(_webengine, SIGNAL(titleChanged()), this, SLOT(_sendData()));

    connect(_webengine, SIGNAL(jsMessage(int, QString, int, QString)), this,
            SLOT(_logJsMessage(int, QString, int, QString)));
}

Webbrowser::~Webbrowser()
{
}

bool Webbrowser::event(QEvent* event_)
{
    // Work around missing key event support in Qt for offscreen windows
    if (auto inputEvent = dynamic_cast<QInputMethodEvent*>(event_))
    {
        if (_webengine->hasFocus())
            return QmlKeyInjector::sendToWebengine(inputEvent, _webengine);
        return QmlKeyInjector::send(inputEvent, _qmlStreamer->getRootItem());
    }
    return QGuiApplication::event(event_);
}

bool _canBeHttpUrl(const QString& url)
{
    return url.startsWith("www.") || (url.contains('.') && !url.contains(' '));
}

void Webbrowser::_processAddressBarInput(const QString url)
{
    auto address = QUrl{url};
    if (address.scheme().isEmpty())
    {
        if (_canBeHttpUrl(url))
            address.setScheme("http");
        else
            address = QUrl{searchUrl.arg(url.toHtmlEscaped())};
    }
    QQmlProperty::write(_webengine, "url", address);
}

WebbrowserHistory _getNavigationHistory(const QQuickItem* webengine)
{
    const auto history = qvariant_cast<QObject*>(
        QQmlProperty::read(webengine, "navigationHistory"));

    const auto itemsModel =
        qvariant_cast<QAbstractListModel*>(history->property("items"));
    const auto itemsCount = itemsModel->rowCount();
    const auto urlRole = itemsModel->roleNames().key("url");

    auto urls = std::vector<QString>();
    urls.reserve(itemsCount);
    for (int i = 0; i < itemsCount; ++i)
        urls.push_back(itemsModel->index(i, 0).data(urlRole).toString());

    const auto backItemsModel =
        qvariant_cast<QAbstractListModel*>(history->property("backItems"));
    const auto currentIndex = size_t(backItemsModel->rowCount());

    return {std::move(urls), currentIndex};
}

void Webbrowser::_sendData()
{
    const auto history = _getNavigationHistory(_webengine);
    const auto title = QQmlProperty::read(_webengine, "title").toString();
    const auto data = WebbrowserContent::serializeData(history, title);
    if (!_qmlStreamer->sendData(data))
        QGuiApplication::quit();
}

void Webbrowser::_logJsMessage(const int level, const QString message,
                               const int lineNumber, const QString sourceID)
{
    const auto line = QString::number(lineNumber);
    const auto msg = QString("%1 @ l.%2 in %3").arg(message, line, sourceID);
    put_log(_getLogLevel(level), LOG_JS, msg.toLocal8Bit().constData());
}
