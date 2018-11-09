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

#include "QmlKeyInjector.h"

#include <QGuiApplication>

bool _isBackspaceEvent(const QInputMethodEvent* inputEvent)
{
    return inputEvent->commitString().isEmpty() &&
           inputEvent->replacementStart() == -1 &&
           inputEvent->replacementLength() == 1;
}

bool _isEnterEvent(const QInputMethodEvent* inputEvent)
{
    return inputEvent->commitString() == "\n";
}

bool _sendEventToWebengine(QEvent* event, QQuickItem* webengineItem)
{
    for (auto child : webengineItem->childItems())
        QGuiApplication::instance()->sendEvent(child, event);
    return event->isAccepted();
}

bool _sendKeyEventToWebengine(const int key, QQuickItem* webengineItem)
{
    QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier);
    QKeyEvent release(QEvent::KeyRelease, key, Qt::NoModifier);
    return _sendEventToWebengine(&press, webengineItem) &&
           _sendEventToWebengine(&release, webengineItem);
}

bool QmlKeyInjector::sendToWebengine(QInputMethodEvent* inputEvent,
                                     QQuickItem* webengineItem)
{
    if (_isEnterEvent(inputEvent))
        return _sendKeyEventToWebengine(Qt::Key_Return, webengineItem);

    if (_isBackspaceEvent(inputEvent))
        return _sendKeyEventToWebengine(Qt::Key_Backspace, webengineItem);

    return _sendEventToWebengine(inputEvent, webengineItem);
}

bool _sendEvent(QEvent* event, QQuickItem* targetItem)
{
    QGuiApplication::instance()->sendEvent(targetItem, event);
    return event->isAccepted();
}

bool _sendKeyEvent(const int key, QQuickItem* rootItem)
{
    QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier);
    QKeyEvent release(QEvent::KeyRelease, key, Qt::NoModifier);
    return _sendEvent(&press, rootItem) && _sendEvent(&release, rootItem);
}

bool QmlKeyInjector::send(QInputMethodEvent* inputEvent, QQuickItem* rootItem)
{
    // Work around missing key event support in Qt for offscreen windows.
    const auto items = rootItem->findChildren<QQuickItem*>();
    for (auto item : items)
    {
        if (!item->hasFocus())
            continue;

        if (_isEnterEvent(inputEvent) && _sendKeyEvent(Qt::Key_Enter, item))
            return true;
        if (_sendEvent(inputEvent, item))
            return true;
    }
    return false;
}
