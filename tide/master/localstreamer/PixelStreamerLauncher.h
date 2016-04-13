/*********************************************************************/
/* Copyright (c) 2013-2015, EPFL/Blue Brain Project                  */
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

#ifndef PIXELSTREAMERLAUNCHER_H
#define PIXELSTREAMERLAUNCHER_H

#include "types.h"

#include <set>

#include <QObject>
#include <QPointF>
#include <QSize>

/**
 * Launch Pixel Streamers as separate processes.
 *
 * The processes connect to the Master application on localhost using the
 * deflect::Stream API. They can be terminated by the user by closing their
 * associated window.
 */
class PixelStreamerLauncher : public QObject
{
    Q_OBJECT

public:
    /**
     * Create a new PixelStreamerLauncher
     *
     * @param windowManager Manages the windows of the streamers
     * @param config The configuration for the default parameters
     */
    PixelStreamerLauncher( PixelStreamWindowManager& windowManager,
                           const MasterConfiguration& config );

    static const QString appLauncherUri;
    static const QString contentLoaderUri;
    static const QString sessionLoaderUri;
    static const QString launcherUri;

public slots:
    /**
     * Open a WebBrowser.
     *
     * @param pos The position of the center of the browser window.
     *        If pos.isNull(), the window is centered on the DisplayWall.
     * @param size The initial size of the viewport of the webbrowser in pixels.
     * @param url The webpage to open.
     */
    void openWebBrowser( QPointF pos, QSize size, QString url );

    /**
     * Open the Dock using default parameters.
     *
     * A new dock instance is created if it was closed, otherwise the existing
     * Dock instance is moved to the given position.
     * @param pos The position of the center of the Dock
     */
    void openDock( QPointF pos );

    /** Hide the Dock. */
    void hideDock();

    /**
     * Open the Content loader.
     * @param pos The position of the top-left corner of the panel
     */
    void openContentLoader( QPointF pos );

    /**
     * Open the Session loader.
     * @param pos The position of the top-left corner of the panel
     */
    void openSessionLoader( QPointF pos );

    /**
     * Open the Applications launcher.
     * @param pos The position of the top-left corner of the panel
     * @return true on success, false on error or if the path to the AppLauncher
     *         QML file is not defined in the configuration
     */
    bool openAppLauncher( QPointF pos );

    /** Open the Qml launcher. */
    void openLauncher();

    /** Hide the Qml Launcher. */
    void hideLauncher();

signals:
    /** Request the launch of a command in a working directory and given ENV. */
    void start( QString command, QString workingDir, QStringList env );

private slots:
    void _dereferenceLocalStreamer( QString uri );

private:
    Q_DISABLE_COPY( PixelStreamerLauncher )

    typedef std::set<QString> Streamers;
    Streamers _processes;

    PixelStreamWindowManager& _windowManager;
    const MasterConfiguration& _config;

    bool _createDock( const QString& uri, const QSize& size,
                      const QString& rootDir );
    QString _getLocalStreamerBin() const;
    QString _getQmlStreamerBin() const;
    QString _getLauncherBin() const;
};

#endif // PIXELSTREAMERLAUNCHER_H
