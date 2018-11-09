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

#include "SessionController.h"

#include "ContentLoader.h"
#include "config.h"
#include "scene/Scene.h"
#include "session/SessionLoader.h"
#include "session/SessionSaver.h"

#if TIDE_ENABLE_WEBBROWSER_SUPPORT
#include "scene/WebbrowserContent.h"
#endif

SessionController::SessionController(Session& session_,
                                     const Configuration::Folders& folders)
    : _session{session_}
    , _folders(folders)
{
    connect(&_loadSessionOp, &QFutureWatcher<Session>::finished, [this]() {
        auto session = _loadSessionOp.result();
        auto scene = session.getScene();
        if (scene)
        {
            _session.assign(session);
#if TIDE_ENABLE_WEBBROWSER_SUPPORT
            _restoreWebbrowsers(*_session.getScene());
#endif
        }
        if (_loadSessionCallback)
            _loadSessionCallback(scene != nullptr);
        _loadSessionCallback = nullptr;
    });

    connect(&_saveSessionOp, &QFutureWatcher<bool>::finished, [this]() {
        const auto success = _saveSessionOp.result();
        if (success)
        {
            _session.setFilepath(_saveFilepath);
            _session.setCurrentFileVersion();
        }
        if (_saveSessionCallback)
            _saveSessionCallback(success);
        _saveSessionCallback = nullptr;
    });

    connect(_session.getScene().get(), &Scene::isEmptyChanged, [this] {
        if (_session.getScene()->isEmpty())
            _session.clearInfo();
    });
}

void SessionController::load(const QString& sessionFile, BoolCallback callback)
{
    _loadSessionOp.waitForFinished();

    _loadSessionCallback = callback;
    SessionLoader loader{_session.getScene()};
    _loadSessionOp.setFuture(loader.load(sessionFile));
}

void SessionController::save(const QString& sessionFile, BoolCallback callback)
{
    _saveSessionOp.waitForFinished();

    _saveSessionCallback = callback;
    _saveFilepath = sessionFile;

    SessionSaver saver(_session.getScene(), _folders.tmp, _folders.upload);
    _saveSessionOp.setFuture(saver.save(sessionFile));
}

void SessionController::_restoreWebbrowsers(const Scene& scene)
{
#if TIDE_ENABLE_WEBBROWSER_SUPPORT
    using WebContent = const WebbrowserContent*;
    for (const auto& window : scene.getWindows())
    {
        if (auto browser = dynamic_cast<WebContent>(window->getContentPtr()))
            emit startWebbrowser(*browser);
    }
#else
    Q_UNUSED(scene);
#endif
}
