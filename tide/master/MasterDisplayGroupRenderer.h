/*********************************************************************/
/* Copyright (c) 2016-2018, EPFL/Blue Brain Project                  */
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

#ifndef MASTERDISPLAYGROUPRENDERER_H
#define MASTERDISPLAYGROUPRENDERER_H

#include "types.h"

#include <QQuickItem>
#include <QUuid>

/**
 * Render a display group in Qml in the master application.
 */
class MasterDisplayGroupRenderer : public QObject
{
    Q_OBJECT

public:
    /** Constructor. */
    MasterDisplayGroupRenderer(DisplayGroupPtr group, QQmlEngine& engine,
                               QQuickItem& parentItem);

    /** Destructor */
    ~MasterDisplayGroupRenderer();

private:
    DisplayGroupPtr _displayGroup;
    std::unique_ptr<DisplayGroupController> _groupController;
    QQmlEngine& _engine;
    std::unique_ptr<QQmlContext> _qmlContext;

    std::unique_ptr<QQuickItem> _displayGroupItem;

    using UuidToWindowMap = std::map<QUuid, std::unique_ptr<QQuickItem>>;
    UuidToWindowMap _uuidToWindowMap;

    void _setContextProperties();
    void _createQmlItem(QQuickItem& parentItem);
    void _addWindows();
    void _watchDisplayGroupUpdates();

    void _add(WindowPtr window);
    void _remove(WindowPtr window);
    void _moveToFront(WindowPtr window);
};

#endif
