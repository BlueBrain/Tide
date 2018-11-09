/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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

#ifndef QMLKEYINJECTOR_H
#define QMLKEYINJECTOR_H

#include <QEvent>
#include <QQuickItem>

/**
 * Helper class to inject Key events into offscreen Qml applications.
 */
class QmlKeyInjector
{
public:
    /**
     * Send an InputMethodEvent (keyboard input) into an offscreen Qml scene.
     *
     * This is a workaround for missing "active focus" support offscreen Qml
     * applications. Users must be aware that the event will be delivered to the
     * first QQuickItem encountered that has focus enabled, which may not be
     * the expected behaviour.
     * @param inputEvent the event to send
     * @param rootItem the root item of the scene.
     * @return true if the event could be delivered
     */
    static bool send(QInputMethodEvent* inputEvent, QQuickItem* rootItem);

    /**
     * Send an InputMethodEvent (keyboard input) to an offscreen WebEngineView.
     *
     * WebEngineView must be treated as a special case because the regular send
     * method does not work for them.
     * @param inputEvent the event to send
     * @param webengineItem a Qml WebEngineView item.
     * @return true if the event could be delivered
     */
    static bool sendToWebengine(QInputMethodEvent* inputEvent,
                                QQuickItem* webengineItem);
};

#endif
