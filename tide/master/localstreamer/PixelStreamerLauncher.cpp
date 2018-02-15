/*********************************************************************/
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
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

#include "config.h"

#include "CommandLineOptions.h"
#include "PixelStreamWindowManager.h"
#include "configuration/Configuration.h"
#include "geometry.h"
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
const QString LAUNCHER_BIN("tideLauncher.exe");
const QString WEBBROWSER_BIN("tideWebbrowser.exe");
const QString WHITEBOARD_BIN("tideWhiteboard.exe");
#else
const QString WHITEBOARD_BIN("tideWhiteboard");
const QString LAUNCHER_BIN("tideLauncher");
const QString WEBBROWSER_BIN("tideWebbrowser");
#endif
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
    return QString("%1/%2 %3").arg(appDir, WEBBROWSER_BIN, args);
}

QString _getWebbrowserCommand(const WebbrowserContent& webbrowser)
{
    CommandLineOptions options;
    options.streamId = webbrowser.getURI();
    options.url = webbrowser.getUrl();
    options.width = webbrowser.width();
    options.height = webbrowser.height();
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
                                             const Configuration& config)
    : _windowManager(manager)
    , _config(config)
{
    connect(&_windowManager, &PixelStreamWindowManager::streamWindowClosed,
            this, &PixelStreamerLauncher::_dereferenceProcess,
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
        size = _config.webbrowser.defaultSize;

    const auto uri = QUuid::createUuid().toString();
    _windowManager.openWindow(uri, pos, size, StreamType::WEBBROWSER);

    auto& content = _windowManager.getWindow(uri)->getContent();
    auto& webbrowser = dynamic_cast<WebbrowserContent&>(content);
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

    const auto pos = _getLauncherPos();
    const auto size = _getLauncherSize();
    _windowManager.openWindow(uri, pos, size, StreamType::LAUNCHER);

    CommandLineOptions options;
    options.streamId = launcherUri;
    options.width = size.width();
    options.height = size.height();
    options.contentsDir = _config.folders.contents;
    options.sessionsDir = _config.folders.sessions;
    const auto command = _getLauncherCommand(options.getCommandLine());

    QStringList env;
    if (!_config.launcher.display.isEmpty())
        env.append(QString("DISPLAY=%1").arg(_config.launcher.display));

    _startProcess(uri, command, env);
}

void PixelStreamerLauncher::openWhiteboard()
{
    static int whiteboardCounter = 0;
    const auto uri = QString("Whiteboard%1").arg(whiteboardCounter++);
    const auto centerPos = _getDefaultWindowPosition();
    const auto size = _config.whiteboard.defaultSize;

    _windowManager.openWindow(uri, centerPos, size, StreamType::WHITEBOARD);

    CommandLineOptions options;
    options.streamId = uri;
    options.saveDir = _config.whiteboard.saveDir;
    const auto command = _getWhiteboardCommand(options.getCommandLine());

    _startProcess(uri, command, {});
}

QSize PixelStreamerLauncher::_getLauncherSize() const
{
    const auto width = 0.25 * getSurface().getTotalWidth();
    const auto size = QSize(width, 0.9 * width);

    const auto minSize = QSize{100, 100};
    const auto maxSize = 0.9 * getSurface().getTotalSize();

    return geometry::constrain(size, minSize, maxSize).toSize();
}

QPointF PixelStreamerLauncher::_getLauncherPos() const
{
    // Position for "standard" walls, slightly above the middle
    const auto x = 0.25 * getSurface().getTotalWidth();
    const auto y = 0.35 * getSurface().getTotalHeight();

    auto rect = QRectF{QPointF(), _getLauncherSize()};
    rect.moveCenter({x, y});

    // If the wall is not tall enough, center vertically to fit
    if (rect.top() < 0.05 * getSurface().getTotalHeight())
        rect.moveCenter({x, 0.5 * getSurface().getTotalHeight()});

    return rect.center();
}

void PixelStreamerLauncher::_dereferenceProcess(const QString uri)
{
    _processes.erase(uri);
}

QPointF PixelStreamerLauncher::_getDefaultWindowPosition() const
{
    return {0.5 * getSurface().getTotalWidth(),
            0.35 * getSurface().getTotalHeight()};
}

void PixelStreamerLauncher::_startProcess(const QString& uri,
                                          const QString& command,
                                          const QStringList& env)
{
    _processes.insert(uri);
    emit start(command, QDir::currentPath(), env);
}

const Surface& PixelStreamerLauncher::getSurface() const
{
    return _config.surfaces[0];
}
