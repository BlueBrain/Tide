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

#ifndef WALLRENDERER_H
#define WALLRENDERER_H

#include "types.h"

#include "BackgroundRenderer.h"
#include "WallRenderContext.h"

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QUuid>

class QQuickItem;

/**
 * Renders all contents on the wall.
 */
class WallRenderer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WallRenderer)

public:
    /**
     * Constructor.
     * @param context for rendering Qml elements.
     * @param parentItem to attach to.
     */
    WallRenderer(WallRenderContext context, QQuickItem& parentItem);

    /** Destructor. */
    ~WallRenderer();

    /** Set background content. */
    void setBackground(BackgroundPtr background);

    /** Set the DisplayGroup to render, replacing the previous one. */
    void setDisplayGroup(DisplayGroupPtr displayGroup);

    /** Set different touchpoint's markers. */
    void setMarkers(MarkersPtr markers);

    /** Set different options used for rendering. */
    void setRenderingOptions(OptionsPtr options);

    /** Set the ScreenLock replacing the previous one. */
    void setScreenLock(ScreenLockPtr lock);

    /** Set status used to notify about inactivity timeout. */
    void setCountdownStatus(CountdownStatusPtr status);

    /** @return true if the renderer requires a redraw. */
    bool needRedraw() const;

public slots:
    /** Increment number of rendered/swapped frames for FPS display. */
    void updateRenderedFrames();

private:
    WallRenderContext _context;
    QQmlContext& _qmlContext;

    BackgroundPtr _background;
    CountdownStatusPtr _countdownStatus;
    DisplayGroupPtr _displayGroup;
    MarkersPtr _markers;
    OptionsPtr _options;
    ScreenLockPtr _screenLock;

    std::unique_ptr<QQuickItem> _surfaceItem;
    std::unique_ptr<BackgroundRenderer> _backgroundRenderer;
    std::unique_ptr<DisplayGroupRenderer> _displayGroupRenderer;

    void _setContextProperties();
    void _createSurfaceItem(QQuickItem& parentItem);
    void _createGroupRenderer();

    void _setBackground(const Background& background);
    bool _hasBackgroundChanged(const QString& newUri) const;
    void _adjustBackgroundTo(const DisplayGroup& displayGroup);
};

#endif
