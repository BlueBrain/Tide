/*********************************************************************/
/* Copyright (c) 2014-2018, EPFL/Blue Brain Project                  */
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

#include "config.h"
#include "types.h"

#include "network/MessageHeader.h"
#include "scene/Window.h"

#include <QMetaType>

/**
 * Register types for use in Qt signals/slots
 */
struct MetaTypeRegistration
{
    MetaTypeRegistration()
    {
        qRegisterMetaType<BackgroundPtr>("BackgroundPtr");
        qRegisterMetaType<BoolCallback>("BoolCallback");
        qRegisterMetaType<WindowPtr>("WindowPtr");
        qRegisterMetaType<WindowPtrs>("WindowPtrs");
        qRegisterMetaType<Window::ResizeHandle>("Window::ResizeHandle");
        qRegisterMetaType<Window::WindowState>("Window::WindowState");
        qRegisterMetaType<ContentSynchronizerSharedPtr>(
            "ContentSynchronizerSharedPtr");
        qRegisterMetaType<CountdownStatusPtr>("CountdownStatusPtr");
        qRegisterMetaType<DisplayGroupPtr>("DisplayGroupPtr");
        qRegisterMetaType<ScenePtr>("ScenePtr");
        qRegisterMetaType<ImagePtr>("ImagePtr");
        qRegisterMetaType<MarkersPtr>("MarkersPtr");
        qRegisterMetaType<MessageType>("MessageType");
        qRegisterMetaType<OptionsPtr>("OptionsPtr");
        qRegisterMetaType<QUuid>("QUuid");
        qRegisterMetaType<ScreenLockPtr>("ScreenLockPtr");
        qRegisterMetaType<std::string>("std::string");
        qRegisterMetaType<TilePtr>("TilePtr");
        qRegisterMetaType<TileWeakPtr>("TileWeakPtr");
        qRegisterMetaTypeStreamOperators<QUuid>("QUuid");
    }
};

// Static instance to register types during library static initialisation phase
static MetaTypeRegistration staticInstance;

Q_DECLARE_METATYPE(QUuid)
