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

#ifndef CONTENTCONTROLLER_H
#define CONTENTCONTROLLER_H

#include "types.h"

#include <QObject>

/**
 * Handle user interaction with the Content of a Window.
 *
 * This class is abstract and should be reimplemented for the
 * different Content type.
 */
class ContentController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ContentController)

public:
    /** Construct a content controller for the given window. */
    static std::unique_ptr<ContentController> create(Window& window);

    /** Construct a default content controller that does nothing. */
    explicit ContentController(Window& window);

    /** Virtual destructor. */
    virtual ~ContentController() = default;

    /** @name Touch gesture handlers. */
    //@{
    Q_INVOKABLE void touchBegin(const QPointF& position);
    Q_INVOKABLE void touchEnd(const QPointF& position);

    Q_INVOKABLE void addTouchPoint(int id, const QPointF& position);
    Q_INVOKABLE void updateTouchPoint(int id, const QPointF& position);
    Q_INVOKABLE void removeTouchPoint(int id, const QPointF& position);

    Q_INVOKABLE void tap(const QPointF& position, uint numPoints);
    Q_INVOKABLE void doubleTap(const QPointF& position, uint numPoints);
    Q_INVOKABLE void tapAndHold(const QPointF& position, uint numPoints);

    Q_INVOKABLE void pan(const QPointF& position, const QPointF& delta,
                         uint numPoints);
    Q_INVOKABLE void pinch(const QPointF& position, const QPointF& pixelDelta);

    Q_INVOKABLE void swipeLeft();
    Q_INVOKABLE void swipeRight();
    Q_INVOKABLE void swipeUp();
    Q_INVOKABLE void swipeDown();
    //@}

    /** @name Keyboard event handlers. */
    //@{
    Q_INVOKABLE void keyPress(int key, int modifiers, const QString& text);
    Q_INVOKABLE void keyRelease(int key, int modifiers, const QString& text);
    //@}

    /** @name UI event handlers. */
    //@{
    Q_INVOKABLE void prevPage();
    Q_INVOKABLE void nextPage();
    //@}

protected:
    Content& getContent();
    const Content& getContent() const;
    QSizeF getWindowSize() const;
    const Window& getWindow() const;

private:
    Window& _window;

    virtual void _touchBegin(const QPointF& position) { Q_UNUSED(position); }
    virtual void _touchEnd(const QPointF& position) { Q_UNUSED(position); }
    virtual void _addTouchPoint(int id, const QPointF& position)
    {
        Q_UNUSED(id);
        Q_UNUSED(position);
    }
    virtual void _updateTouchPoint(int id, const QPointF& position)
    {
        Q_UNUSED(id);
        Q_UNUSED(position);
    }
    virtual void _removeTouchPoint(int id, const QPointF& position)
    {
        Q_UNUSED(id);
        Q_UNUSED(position);
    }

    virtual void _tap(const QPointF& position, uint numPoints)
    {
        Q_UNUSED(position);
        Q_UNUSED(numPoints);
    }
    virtual void _doubleTap(const QPointF& position, uint numPoints)
    {
        Q_UNUSED(position);
        Q_UNUSED(numPoints);
    }
    virtual void _tapAndHold(const QPointF& position, uint numPoints)
    {
        Q_UNUSED(position);
        Q_UNUSED(numPoints);
    }
    virtual void _pan(const QPointF& position, const QPointF& delta,
                      uint numPoints)
    {
        Q_UNUSED(position);
        Q_UNUSED(delta);
        Q_UNUSED(numPoints);
    }
    virtual void _pinch(const QPointF& position, const QPointF& pixelDelta)
    {
        Q_UNUSED(position);
        Q_UNUSED(pixelDelta);
    }
    virtual void _swipeLeft() {}
    virtual void _swipeRight() {}
    virtual void _swipeUp() {}
    virtual void _swipeDown() {}
    /** @name Keyboard event handlers. */
    //@{
    virtual void _keyPress(int key, int modifiers, const QString& text)
    {
        Q_UNUSED(key);
        Q_UNUSED(modifiers);
        Q_UNUSED(text);
    }
    virtual void _keyRelease(int key, int modifiers, const QString& text)
    {
        Q_UNUSED(key);
        Q_UNUSED(modifiers);
        Q_UNUSED(text);
    }
    //@}

    /** @name UI event handlers. */
    //@{
    virtual void _prevPage() {}
    virtual void _nextPage() {}
    //@}
};

#endif
