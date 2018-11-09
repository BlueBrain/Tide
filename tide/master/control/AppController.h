/*********************************************************************/
/* Copyright (c) 2018, EPFL/Blue Brain Project                       */
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

#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include "types.h"

#include "config.h"

#include <QObject>

class PixelStreamerLauncher;
class SceneController;
class SessionController;
class ScreenController;

/**
 * The application's main controller.
 */
class AppController : public QObject
{
    Q_OBJECT

public:
    AppController(Session& session, ScreenLock& lock,
                  deflect::server::Server& deflectServer, Options& options,
                  const Configuration& config);
    ~AppController();

    /** Open a content. */
    void open(uint surfaceIndex, QString uri, QPointF coords,
              BoolCallback callback);

    /** Open a list of contents. */
    void openAll(const QStringList& uris);

    /** Clear all contents from a surface. */
    void clear(uint surfaceIndex);

    /** Load a session. */
    void load(QString uri, BoolCallback callback);

    /** Save a session to the given file. */
    void save(QString uri, BoolCallback callback);

    /** Open the launcher. */
    void openLauncher();

    /**
     * Open a web browser.
     *
     * @param surfaceIndex The surface on which to open the window.
     * @param url The webpage to open.
     * @param size The initial size of the viewport in pixels.
     * @param pos The position of the center of the browser window.
     *        If pos.isNull(), the window is centered on the DisplayWall.
     * @param debugPort Optional port to enable Chromium's remote debugging.
     */
    void openWebbrowser(uint surfaceIndex, QString url, QSize size, QPointF pos,
                        ushort debugPort);

    /** Open a whiteboard. */
    void openWhiteboard(uint surfaceIndex);

    /** Terminate a pixel stream. */
    void terminateStream(const QString& uri);

    /** Suspend activity (turn off screens). */
    void suspend(BoolCallback callback);

    /** Resumue activity (turn on screens). */
    void resume(BoolCallback callback);

    /** Resume activity when a touch occurs. */
    void handleTouchEvent(uint numTouchPoints);

signals:
    /** Emitted when the power state of the displays changes. */
    void screenStateChanged(ScreenState state);

    /** Emitted when the state of the countdown is modified. */
    void countdownUpdated(CountdownStatusPtr);

    /** Request the launch of a command in a working directory and given ENV. */
    void start(QString command, QString workingDir, QStringList env);

private:
    std::unique_ptr<SceneController> _sceneController;
    std::unique_ptr<SessionController> _sessionController;
    std::unique_ptr<PixelStreamWindowManager> _pixelStreamWindowManager;
    std::unique_ptr<PixelStreamerLauncher> _pixelStreamerLauncher;
    ScreenLock& _lock;
    const Configuration& _config;

    std::unique_ptr<InactivityTimer> _inactivityTimer;
#if TIDE_ENABLE_PLANAR_CONTROLLER
    std::unique_ptr<ScreenController> _screenController;
#endif

    void _initScreenController(const Configuration& config);
    void _connect(ScreenLock& lock,
                  PixelStreamWindowManager& pixelStreamWindowManager);
    void _connect(deflect::server::Server& deflectServer,
                  PixelStreamWindowManager& pixelStreamWindowManager);
    void _connect(Options& options,
                  PixelStreamWindowManager& pixelStreamWindowManager);
};

#endif
