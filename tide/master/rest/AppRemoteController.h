/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
/*                          Pawel Podhajski <pawel.podhajski@epfl.ch>*/
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

#ifndef APP_REMOTE_CONTROLLER_H
#define APP_REMOTE_CONTROLLER_H

#include "types.h"

#include <rockets/jsonrpc/asyncReceiver.h>

#include <QObject>

/**
 * Remote controller for the application using JSON-RPC.
 */
class AppRemoteController : public QObject,
                            private rockets::jsonrpc::AsyncReceiver
{
    Q_OBJECT

public:
    /**
     * Construct an application controller.
     *
     * @param config the application configuration.
     */
    AppRemoteController(const Configuration& config);

    using rockets::jsonrpc::AsyncReceiver::process;

signals:
    /** Open a content. */
    void open(uint surfaceIndex, QString uri, QPointF coords,
              BoolMsgCallback callback);

    /** Load a session. */
    void load(QString uri, BoolCallback callback);

    /** Save a session to the given file. */
    void save(QString uri, BoolCallback callback);

    /** Browse a website. */
    void browse(uint surfaceIndex, QString url, QSize size, QPointF pos,
                ushort debugPort);

    /** Open a whiteboard. */
    void openWhiteboard(uint surfaceIndex);

    /** Take a screenshot. */
    void takeScreenshot(uint surfaceIndex, QString filename);

    /** Power off the screens. */
    void powerOff(BoolCallback callback);

    /** Power on the screens. */
    void powerOn(BoolCallback callback);

    /** Exit the application. */
    void exit();
};

#endif
