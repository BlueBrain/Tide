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

#ifndef MASTERWINDOW_H
#define MASTERWINDOW_H

#include "types.h"

#include <QFutureWatcher>
#include <QMainWindow>
#include <QMimeData>

class BackgroundWidget;
class MasterQuickView;
class WebbrowserWidget;

/**
 * The main UI window for Master applications.
 *
 * It lets users control the contents displayed on the wall.
 */
class MasterWindow : public QMainWindow
{
    Q_OBJECT

public:
    /** Constructor. */
    MasterWindow(ScenePtr scene, OptionsPtr options, Configuration& config);

    /** @return the quick view(s). */
    std::vector<MasterQuickView*> getQuickViews();

signals:
    /** Load a session. */
    void load(QString uri, BoolCallback callback);

    /** Save a session to the given file. */
    void save(QString uri, BoolCallback callback);

    /** Emitted when users want to open a webbrowser. */
    void openWebBrowser(uint surfaceIndex, QString url, QSize size, QPointF pos,
                        ushort debugPort);

    /** Emitted when users want to open a whiteboard. */
    void openWhiteboard(uint surfaceIndex);

private:
    /** @name Drag events re-implemented from QMainWindow */
    //@{
    void dragEnterEvent(QDragEnterEvent* event) final;
    void dropEvent(QDropEvent* event) final;
    //@}

    void _setupMasterWindowUI();
    void _addSurfacesTabViews(const Configuration& config);
    void _addDisplayGroupTabView(std::unique_ptr<MasterQuickView> quickView,
                                 const QString& title);

    void _openContent();
    void _addContentDirectory(const QString& directoryName,
                              const QSize& gridSize = QSize());
    void _openContentsDirectory();

    void _openSession();
    void _saveSession();
    void _loadSession(const QString& filename);

    void _openAboutWidget();

    uint _getActiveSceneIndex() const;
    DisplayGroup& _getActiveGroup();

    QStringList _extractValidContentUrls(const QMimeData* mimeData);
    QStringList _extractFolderUrls(const QMimeData* mimeData);
    QString _extractSessionFile(const QMimeData* mimeData);

    ScenePtr _scene;
    OptionsPtr _options;

    BackgroundWidget* _backgroundWidget;       // child QObject
    WebbrowserWidget* _webbrowserWidget;       // child QObject
    std::vector<MasterQuickView*> _quickViews; // child QObjects

    QString _contentFolder;
    QString _sessionFolder;
};

#endif
