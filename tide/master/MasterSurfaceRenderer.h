/*********************************************************************/
/* Copyright (c) 2017-2018, EPFL/Blue Brain Project                  */
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

#ifndef MASTERSURFACERENDERER_H
#define MASTERSURFACERENDERER_H

#include "types.h"

#include <QQuickItem>

class MasterDisplayGroupRenderer;

/**
 * Render a surface in Qml in the master application.
 */
class MasterSurfaceRenderer : public QObject
{
    Q_OBJECT

public:
    /** Constructor. */
    MasterSurfaceRenderer(size_t surfaceIndex, DisplayGroupPtr group,
                          QQmlEngine& engine, QQuickItem& parentItem);

    /** Destructor */
    ~MasterSurfaceRenderer();

signals:
    /** Emitted when a user taps the launcher control to open it. */
    void openLauncher();

private:
    DisplayGroupPtr _group;
    std::unique_ptr<DisplayGroupController> _groupController;
    std::unique_ptr<QQuickItem> _surfaceItem;
    std::unique_ptr<MasterDisplayGroupRenderer> _displayGroupRenderer;

    void _setContextProperties(QQmlContext& context);
    void _createSurfaceItem(QQmlEngine& engine);
    void _createBasicSurfaceItem(QQmlEngine& engine);
    void _createGroupRenderer(QQmlEngine& engine);
};

#endif
