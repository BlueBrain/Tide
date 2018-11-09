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

#ifndef MULTI_CHANNEL_WINDOW_CONTROLLER_H
#define MULTI_CHANNEL_WINDOW_CONTROLLER_H

#include "types.h"

#include <QObject>

/**
 * Monitor the Scene to apply window operations such as fullscreen / exit
 * fullscreen / close to windows on all surfaces for multi-channel contents.
 */
class MultiChannelWindowController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(MultiChannelWindowController)

public:
    /**
     * Handle multi-channel content window operations on all surfaces.
     *
     * @param scene to control.
     */
    MultiChannelWindowController(Scene& scene);

    void closeSingleWindow(const QString& uri, const QUuid& windowId);
    void closeAllWindows(const QString& uri);

private:
    Scene& _scene;
    std::map<QString, std::set<QUuid>> _contentToWindowsMap;

    void _monitor(const DisplayGroup& group);
    void _onWindowAdded(WindowPtr window);
    void _onWindowRemoved(WindowPtr window);

    void _changeFullscreenModeForAllWindows(const QString& uri,
                                            bool fullscreenMode);
    void _changeFullscreenMode(const QUuid& windowId, bool fullscreenMode);

    void _closeAll(const std::set<QUuid> windows);
    void _close(const QUuid& windowId);
};

#endif
