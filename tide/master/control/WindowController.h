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

#ifndef WINDOWCONTROLLER_H
#define WINDOWCONTROLLER_H

#include "types.h"

#include <QObject>

/** Common window size states. */
enum SizeState
{
    SIZE_1TO1,
    SIZE_1TO1_FITTING,
    SIZE_LARGE,
    SIZE_FULLSCREEN,
    SIZE_FULLSCREEN_MAX,
    SIZE_FULLSCREEN_1TO1
};

/** Window point, used for affine transforms of the window. */
enum WindowPoint
{
    TOP_LEFT,
    CENTER
};

/**
 * Controller for moving and resizing windows.
 */
class WindowController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WindowController)

public:
    /**
     * The set of window coordinates that the controller acts upon.
     */
    enum class Coordinates
    {
        STANDARD,   // standard coordinates
        FOCUSED,    // focused coordinates
        FULLSCREEN, // fullscreen coordinates
        AUTO        // current window mode's coordinates
    };

    /**
     * Create a controller for a window.
     * @param window the target window
     * @param group the group to which the window belongs
     * @param target the set of coordinates to modify (default: AUTO)
     */
    WindowController(Window& window, const DisplayGroup& group,
                     Coordinates target = Coordinates::AUTO);

    /** Resize the window. */
    Q_INVOKABLE void resize(QSizeF size, WindowPoint fixedPoint = TOP_LEFT);

    /** Resize the window relative to the current active window border */
    Q_INVOKABLE void resizeRelative(const QPointF& delta);

    /** Scale the window by the given pixel delta (around the given center). */
    Q_INVOKABLE void scale(const QPointF& center, double pixelDelta);
    Q_INVOKABLE void scale(const QPointF& center, const QPointF& pixelDelta);

    /** Adjust the window coordinates to match the desired state. */
    void adjustSize(const SizeState state);

    /** Toggle between SIZE_FULLSCREEN and SIZE_FULLSCREEN_MAX. */
    Q_INVOKABLE void toogleFullscreenMaxSize();

    /** Toggle the selected state of the window. */
    Q_INVOKABLE void toggleSelected();

    /** Move the window to the desired position. */
    Q_INVOKABLE void moveTo(const QPointF& position,
                            WindowPoint handle = TOP_LEFT);

    /** Move the center of the window to the desired position. */
    inline void moveCenterTo(const QPointF& position)
    {
        moveTo(position, CENTER);
    }

    /** Move the window by the given delta. */
    Q_INVOKABLE void moveBy(const QPointF& delta);

    /** @return the minimum size of the window, 5% of wall size or 300px. */
    QSizeF getMinSize() const;

    /** @return the maximum size of the window, considering max size of content,
     *          wall size and current content zoom. */
    QSizeF getMaxSize() const;

    /** Constrain the given size between getMinSize() and getMaxSize(). */
    void constrainSize(QSizeF& windowSize) const;

    /** @return the minimum size of the window respecting its aspect ratio. */
    QSizeF getMinSizeAspectRatioCorrect() const;

private:
    /**
     * Resize the window around a given center point.
     * @param center the center of scaling
     * @param size the new desired size
     */
    void _resize(const QPointF& center, QSizeF size);

    void _constrainAspectRatio(QSizeF& windowSize) const;
    bool _isCloseToContentAspectRatio(const QSizeF& windowSize) const;
    void _snapToContentAspectRatio(QSizeF& windowSize) const;
    void _constrainPosition(QRectF& window) const;
    QRectF _getCenteredCoordinates(const QSizeF& size) const;

    bool _targetIsFullscreen() const;
    const QRectF& _getCoordinates() const;
    void _apply(const QRectF& coordinates);

    Window& _window;
    const DisplayGroup& _displayGroup;
    Coordinates _target;
};

#endif
