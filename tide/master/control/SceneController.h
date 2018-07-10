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

#ifndef SCENECONTROLLER_H
#define SCENECONTROLLER_H

#include "types.h"

#include "configuration/Configuration.h"
#include "control/WindowController.h"

#include <QFutureWatcher>
#include <QObject>

/**
 * Controller for all Scene operations.
 */
class SceneController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SceneController);

public:
    /** Constructor */
    SceneController(Scene& scene, const Configuration::Folders& folders);

    void openAll(const QStringList& uris);

    void open(uint surfaceIndex, const QString& uri, const QPointF& coords,
              BoolCallback callback);
    void load(const QString& sessionFile, BoolCallback callback);
    void save(const QString& sessionFile, BoolCallback callback);

    /** Replace the contents by that of another scene. */
    void apply(SceneConstPtr scene);

    /** Hide the Launcher. */
    void hideLauncher();

    std::unique_ptr<WindowController> getController(const QUuid& winId);

signals:
    void startWebbrowser(const WebbrowserContent& browser);

private:
    Scene& _scene;
    Configuration::Folders _folders;

    QFutureWatcher<ScenePtr> _loadSessionOp;
    QFutureWatcher<bool> _saveSessionOp;
    BoolCallback _loadSessionCallback;
    BoolCallback _saveSessionCallback;

    void _restoreWebbrowsers(const Scene& scene);
    void _deleteTempContentFile(WindowPtr window);
};

#endif
