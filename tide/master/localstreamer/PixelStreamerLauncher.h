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
    Q_DISABLE_COPY(PixelStreamerLauncher)

public:
    /**
     * Create a new PixelStreamerLauncher
     *
     * @param manager Manages the windows of the streamers
     * @param config The configuration for the default parameters
     */
    PixelStreamerLauncher(PixelStreamWindowManager& manager,
                          const Configuration& config);

    static const QString launcherUri;

public slots:
    /**
     * Open a web browser.
     *
     * @param surfaceIndex The surface on which to open the window.
     * @param url The webpage to open.
     * @param size The initial size of the viewport in pixels.
     * @param pos The position of the center of the window.
     *        If pos.isNull(), the window is centered on the DisplayWall.
     * @param debugPort Optional port to enable Chromium's remote debugging.
     */
    void openWebbrowser(uint surfaceIndex, QString url, QSize size, QPointF pos,
                        ushort debugPort);

    /** Start a web browser for an existing window. */
    void launch(const WebbrowserContent& webbrowser, ushort debugPort = 0u);

    /** Open the Qml launcher. */
    void openLauncher();

    /** Open the Qml whiteboard. */
    void openWhiteboard(uint surfaceIndex);

signals:
    /** Request the launch of a command in a working directory and given ENV. */
    void start(QString command, QString workingDir, QStringList env);

private:
    PixelStreamWindowManager& _windowManager;
    const Configuration& _config;
    std::set<QString> _processes;

    void _startProcess(const QString& uri, const QString& command,
                       const QStringList& env);
    void _dereferenceProcess(QString uri);
};

#endif
