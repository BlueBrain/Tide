/*********************************************************************/
/* Copyright (c) 2013-2017, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
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

#include "PixelStreamerLauncher.h"

#include "CommandLineOptions.h"
#include "MasterConfiguration.h"
#include "PixelStreamWindowManager.h"
#include "log.h"
#include "scene/ContentType.h"
#include "scene/ContentWindow.h"
#if TIDE_ENABLE_WEBBROWSER_SUPPORT
#include "scene/WebbrowserContent.h"
#endif

#include <QCoreApplication>
#include <QDir>

namespace
{
#ifdef _WIN32
const QString LOCALSTREAMER_BIN("tideLocalstreamer.exe");
const QString LAUNCHER_BIN("tideLauncher.exe");
const QString WEBBROWSER_BIN("tideWebbrowser.exe");
const QString WHITEBOARD_BIN("tideWhiteboard.exe");
#else
const QString LOCALSTREAMER_BIN("tideLocalstreamer");
const QString WHITEBOARD_BIN("tideWhiteboard");
const QString LAUNCHER_BIN("tideLauncher");
const QString WEBBROWSER_BIN("tideWebbrowser");
#endif

#if TIDE_ENABLE_WEBBROWSER_SUPPORT
const QSize WEBBROWSER_DEFAULT_SIZE(1280, 1024);
#endif
const QSize WHITEBOARD_DEFAULT_SIZE(1920, 1080);
}

QString _getLauncherCommand(const QString& args)
{
    const auto appDir = QCoreApplication::applicationDirPath();
    return QString("%1/%2 %3").arg(appDir, LAUNCHER_BIN, args);
}

#if TIDE_ENABLE_WEBBROWSER_SUPPORT
QString _getWebbrowserCommand(const QString& args)
{
    const auto appDir = QCoreApplication::applicationDirPath();
#ifdef TIDE_USE_QT5WEBENGINE
    const auto& app = WEBBROWSER_BIN;
#else
    const auto& app = LOCALSTREAMER_BIN;
#endif
    return QString("%1/%2 %3").arg(appDir, app, args);
}

QString _getWebbrowserCommand(const WebbrowserContent& webbrowser)
{
    CommandLineOptions options;
    options.setStreamId(webbrowser.getURI());
    options.setUrl(webbrowser.getUrl());
    options.setWidth(webbrowser.width());
    options.setHeight(webbrowser.height());
#ifdef TIDE_USE_QT5WEBKITWIDGETS
    options.setPixelStreamerType(PS_WEBKIT);
#endif
    return _getWebbrowserCommand(options.getCommandLine());
}

QStringList _getWebbrowserEnv(const ushort debugPort)
{
    // The env variable must always be present otherwise it is not reset.
    auto env = QString{"QTWEBENGINE_REMOTE_DEBUGGING="};
    if (debugPort)
        env.append(QString::number((uint)debugPort));
    return QStringList{env};
}
#endif

QString _getWhiteboardCommand(const QString& args)
{
    const auto appDir = QCoreApplication::applicationDirPath();
    return QString("%1/%2 %3").arg(appDir, WHITEBOARD_BIN, args);
}

const QString PixelStreamerLauncher::launcherUri = QString("Launcher");

PixelStreamerLauncher::PixelStreamerLauncher(PixelStreamWindowManager& manager,
                                             const MasterConfiguration& config)
    : _windowManager(manager)
    , _config(config)
{
    connect(&_windowManager, &PixelStreamWindowManager::streamWindowClosed,
            this, &PixelStreamerLauncher::_dereferenceLocalStreamer,
            Qt::QueuedConnection);
}

void PixelStreamerLauncher::openWebBrowser(QPointF pos, QSize size,
                                           const QString url,
                                           const ushort debugPort)
{
#if TIDE_ENABLE_WEBBROWSER_SUPPORT
    if (pos.isNull())
        pos = _getDefaultWindowPosition();
    if (size.isEmpty())
        size = WEBBROWSER_DEFAULT_SIZE;

    const auto uri = QUuid::createUuid().toString();
    _windowManager.openWindow(uri, pos, size, StreamType::WEBBROWSER);

    auto content = _windowManager.getWindow(uri)->getContent();
    auto& webbrowser = dynamic_cast<WebbrowserContent&>(*content);
    webbrowser.setUrl(url);
    launch(webbrowser, debugPort);
#else
    Q_UNUSED(pos);
    Q_UNUSED(size);
    Q_UNUSED(url);
    Q_UNUSED(debugPort);
#endif
}

void PixelStreamerLauncher::launch(const WebbrowserContent& webbrowser,
                                   const ushort debugPort)
{
#if TIDE_ENABLE_WEBBROWSER_SUPPORT
    const auto cmd = _getWebbrowserCommand(webbrowser);
    const auto env = _getWebbrowserEnv(debugPort);
    _startProcess(webbrowser.getURI(), cmd, env);
#else
    Q_UNUSED(webbrowser)
    Q_UNUSED(debugPort)
#endif
}

void PixelStreamerLauncher::openLauncher()
{
    const QString& uri = launcherUri;
    if (_processes.count(uri))
    {
        _windowManager.showWindow(uri);
        return;
    }

    const qreal width = 0.25 * _config.getTotalWidth();
    const QSize launcherSize(width, 0.9 * width);
    const qreal x = 0.25 * _config.getTotalWidth();
    const qreal y = 0.35 * _config.getTotalHeight();
    const QPointF centerPos(x, y);

    _windowManager.openWindow(uri, centerPos, launcherSize,
                              StreamType::LAUNCHER);

    CommandLineOptions options;
    options.setStreamId(launcherUri);
    options.setConfiguration(_config.getFilename());
    options.setWidth(launcherSize.width());
    options.setHeight(launcherSize.height());
    const auto command = _getLauncherCommand(options.getCommandLine());

    QStringList env;
    if (!_config.getLauncherDisplay().isEmpty())
        env.append(QString("DISPLAY=%1").arg(_config.getLauncherDisplay()));

    _startProcess(uri, command, env);
}

void PixelStreamerLauncher::openWhiteboard()
{
    static int whiteboardCounter = 0;
    const auto uri = QString("Whiteboard%1").arg(whiteboardCounter++);
    const auto centerPos = _getDefaultWindowPosition();
    const auto size = WHITEBOARD_DEFAULT_SIZE;

    _windowManager.openWindow(uri, centerPos, size, StreamType::WHITEBOARD);

    CommandLineOptions options;
    options.setStreamId(uri);
    options.setConfiguration(_config.getFilename());
    const auto command = _getWhiteboardCommand(options.getCommandLine());

    _startProcess(uri, command, {});
}

void PixelStreamerLauncher::_dereferenceLocalStreamer(const QString uri)
{
    _processes.erase(uri);
}

QPointF PixelStreamerLauncher::_getDefaultWindowPosition() const
{
    return {0.5 * _config.getTotalWidth(), 0.35 * _config.getTotalHeight()};
}

void PixelStreamerLauncher::_startProcess(const QString& uri,
                                          const QString& command,
                                          const QStringList& env)
{
    _processes.insert(uri);
    emit start(command, QDir::currentPath(), env);
}
