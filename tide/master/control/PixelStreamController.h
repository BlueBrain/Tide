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

#ifndef PIXELSTREAMCONTROLLER_H
#define PIXELSTREAMCONTROLLER_H

#include "ContentController.h"
#include "KeyboardController.h"

#include <deflect/Event.h>

/**
 * Forward user actions to a deflect::Stream using Deflect events.
 */
class PixelStreamController : public ContentController
{
    Q_OBJECT
    Q_PROPERTY(QObject* keyboard READ getKeyboardController CONSTANT)

public:
    /** Constructor */
    explicit PixelStreamController(Window& window);

    QObject* getKeyboardController();

signals:
    /** Emitted when an Event occured. */
    void notify(const deflect::Event& event);

private:
    /** @name Touch gesture handlers. */
    //@{
    void _touchBegin(const QPointF& position) override;
    void _touchEnd(const QPointF& position) override;

    void _addTouchPoint(int id, const QPointF& position) override;
    void _updateTouchPoint(int id, const QPointF& position) override;
    void _removeTouchPoint(int id, const QPointF& position) override;

    void _tap(const QPointF& position, uint numPoints) override;
    void _doubleTap(const QPointF& position, uint numPoints) override;
    void _tapAndHold(const QPointF& position, uint numPoints) override;
    void _pan(const QPointF& position, const QPointF& delta,
              uint numPoints) override;
    void _pinch(const QPointF& position, const QPointF& delta) override;

    void _swipeLeft() override;
    void _swipeRight() override;
    void _swipeUp() override;
    void _swipeDown() override;
    //@}

    /** @name Keyboard event handlers. */
    //@{
    void _keyPress(int key, int modifiers, const QString& text) override;
    void _keyRelease(int key, int modifiers, const QString& text) override;
    //@}

    /** @name UI event handlers. */
    //@{
    void _prevPage() override;
    void _nextPage() override;
    //@}

    PixelStreamContent& _getPixelStreamContent();

    void _sendSizeChangedEvent();

    QPointF _normalize(const QPointF& point) const;
    deflect::Event _getNormEvent(const QPointF& position) const;

    KeyboardController _keyboardController;
};

#endif
