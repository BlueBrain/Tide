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

#include "PixelStreamerLauncher.h"

#include "config.h"

#include "CommandLineOptions.h"
#include "LauncherPlacer.h"
#include "configuration/Configuration.h"
#include "control/PixelStreamWindowManager.h"
#include "scene/ContentType.h"
#include "scene/Window.h"
#include "utils/log.h"
#if TIDE_ENABLE_WEBBROWSER_SUPPORT
#include "scene/WebbrowserContent.h"
#endif

#include <QCoreApplication>
#include <QDir>

namespace
{
const QString LAUNCHER_BIN("tideLauncher");
const QString WEBBROWSER_BIN("tideWebbrowser");
const QString WHITEBOARD_BIN("tideWhiteboard");

QString _getAppsDir()
{
#ifdef __APPLE__
    return QCoreApplication::applicationDirPath() + "/../../.."; // app bundle
#else
    return QCoreApplication::applicationDirPath();
#endif
}
} // namespace

QString _getLauncherCommand(const QString& args)
{
    return QString("%1/%2 %3").arg(_getAppsDir(), LAUNCHER_BIN, args);
}

#if TIDE_ENABLE_WEBBROWSER_SUPPORT
QString _getWebbrowserCommand(const QString& args)
{
    return QString("%1/%2 %3").arg(_getAppsDir(), WEBBROWSER_BIN, args);
}

QString _getWebbrowserCommand(const WebbrowserContent& webbrowser)
{
    CommandLineOptions options;
    options.streamId = webbrowser.getUri();
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
    return QString("%1/%2 %3").arg(_getAppsDir(), WHITEBOARD_BIN, args);
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

void PixelStreamerLauncher::openWebbrowser(const uint surfaceIndex,
                                           const QString url, QSize size,
                                           const QPointF pos,
                                           const ushort debugPort)
{
#if TIDE_ENABLE_WEBBROWSER_SUPPORT
    if (size.isEmpty())
        size = _config.webbrowser.defaultSize;

    const auto uri = QUuid::createUuid().toString();
    _windowManager.openWindow(surfaceIndex, uri, size, pos,
                              StreamType::WEBBROWSER);

    auto& content = _windowManager.getWindows(uri)[0]->getContent();
    auto& webbrowser = dynamic_cast<WebbrowserContent&>(content);
    webbrowser.setUrl(url);
    launch(webbrowser, debugPort);
#else
    Q_UNUSED(surfaceIndex);
    Q_UNUSED(url);
    Q_UNUSED(size);
    Q_UNUSED(pos);
    Q_UNUSED(debugPort);
    print_log(
        LOG_INFO, LOG_GENERAL,
        "Can't browse url '%s', Tide was compiled without web browser support",
        url.toLocal8Bit().constData());
#endif
}

void PixelStreamerLauncher::launch(const WebbrowserContent& webbrowser,
                                   const ushort debugPort)
{
#if TIDE_ENABLE_WEBBROWSER_SUPPORT
    const auto cmd = _getWebbrowserCommand(webbrowser);
    const auto env = _getWebbrowserEnv(debugPort);
    _startProcess(webbrowser.getUri(), cmd, env);
#else
    Q_UNUSED(webbrowser);
    Q_UNUSED(debugPort);
#endif
}

void PixelStreamerLauncher::openLauncher()
{
    const auto& uri = launcherUri;
    if (_processes.count(uri))
    {
        _windowManager.showWindows(uri);
        return;
    }

    const auto rect = LauncherPlacer{_config.surfaces[0]}.getCoordinates();
    const auto size = rect.size();
    const auto pos = rect.center();
    _windowManager.openWindow(0, uri, size, pos, StreamType::LAUNCHER);

    CommandLineOptions options;
    options.streamId = uri;
    options.width = size.width();
    options.height = size.height();
    options.contentsDir = _config.folders.contents;
    options.sessionsDir = _config.folders.sessions;
    options.webservicePort = _config.master.webservicePort;
    options.targetFPS = _config.launcher.targetFPS;
    options.demoServiceUrl = _config.launcher.demoServiceUrl;
#if TIDE_ENABLE_PLANAR_CONTROLLER
    options.showPowerButton = !_config.master.planarSerialPort.isEmpty();
#endif

    const auto command = _getLauncherCommand(options.getCommandLine());

    QStringList env;
    if (!_config.launcher.display.isEmpty())
        env.append(QString("DISPLAY=%1").arg(_config.launcher.display));

    _startProcess(uri, command, env);
}

void PixelStreamerLauncher::openWhiteboard(const uint surfaceIndex)
{
    static int whiteboardCounter = 0;
    const auto uri = QString("Whiteboard%1").arg(whiteboardCounter++);
    const auto size = _config.whiteboard.defaultSize;

    _windowManager.openWindow(surfaceIndex, uri, size, QPointF(),
                              StreamType::WHITEBOARD);

    CommandLineOptions options;
    options.streamId = uri;
    options.saveDir = _config.whiteboard.saveDir;
    const auto command = _getWhiteboardCommand(options.getCommandLine());

    _startProcess(uri, command, {});
}

void PixelStreamerLauncher::_startProcess(const QString& uri,
                                          const QString& command,
                                          const QStringList& env)
{
    _processes.insert(uri);
    emit start(command, QDir::currentPath(), env);
}

void PixelStreamerLauncher::_dereferenceProcess(const QString uri)
{
    _processes.erase(uri);
}
