/*********************************************************************/
/* Copyright (c) 2014-2016, EPFL/Blue Brain Project                  */
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

#include "MasterWindow.h"

#include "BackgroundWidget.h"
#include "ContentLoader.h"
#include "DisplayGroupListWidget.h"
#include "log.h"
#include "MasterConfiguration.h"
#include "MasterQuickView.h"
#include "scene/ContentFactory.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"
#include "scene/Options.h"
#include "StateSerializationHelper.h"
#if TIDE_USE_QT5WEBKITWIDGETS || TIDE_USE_QT5WEBENGINE
#  include "WebbrowserWidget.h"
#endif

#include <tide/core/version.h>

#include <sstream>
#include <QtWidgets>

namespace
{
const QString SESSION_FILES_FILTER( "Session files (*.dcx)" );
const QSize DEFAULT_WINDOW_SIZE( 800, 600 );
}

MasterWindow::MasterWindow( DisplayGroupPtr displayGroup,
                            OptionsPtr options,
                            MasterConfiguration& config )
    : QMainWindow()
    , _displayGroup( displayGroup )
    , _options( options )
    , _backgroundWidget( new BackgroundWidget( config, this ))
#if TIDE_USE_QT5WEBKITWIDGETS || TIDE_USE_QT5WEBENGINE
    , _webbrowserWidget( new WebbrowserWidget( config, this ))
#endif
    , _contentFolder( config.getContentDir( ))
    , _sessionFolder( config.getSessionsDir( ))
{
    _backgroundWidget->setModal( true );

    connect( _backgroundWidget, &BackgroundWidget::backgroundColorChanged,
             _options.get(), &Options::setBackgroundColor );
    connect( _backgroundWidget, &BackgroundWidget::backgroundContentChanged,
             _options.get(), &Options::setBackgroundContent );

#if TIDE_USE_QT5WEBKITWIDGETS || TIDE_USE_QT5WEBENGINE
    connect( _webbrowserWidget, &WebbrowserWidget::openWebBrowser,
             this, &MasterWindow::openWebBrowser );
#endif

    connect( &_loadSessionOp, &QFutureWatcher<DisplayGroupConstPtr>::finished,
             [this]()
    {
        if( auto group = _loadSessionOp.result( ))
            emit sessionLoaded( group );
        else
            QMessageBox::warning( this, "Error", "Could not load session file.",
                                  QMessageBox::Ok, QMessageBox::Ok );
    });

    connect( &_saveSessionOp, &QFutureWatcher<bool>::finished, [this]() {
        if( !_saveSessionOp.result( ))
            QMessageBox::warning( this, "Error", "Could not save session file.",
                                  QMessageBox::Ok, QMessageBox::Ok );
    });

    resize( DEFAULT_WINDOW_SIZE );
    setAcceptDrops( true );

    _setupMasterWindowUI( make_unique<MasterQuickView>( _options, config ));

    show();
}

MasterQuickView* MasterWindow::getQuickView()
{
    return _quickView;
}

void MasterWindow::_setupMasterWindowUI( std::unique_ptr<MasterQuickView>
                                         quickView )
{
    // create menus in menu bar
    QMenu* fileMenu = menuBar()->addMenu( "&File" );
    QMenu* editMenu = menuBar()->addMenu( "&Edit" );
    QMenu* viewMenu = menuBar()->addMenu( "&View" );
    QMenu* helpMenu = menuBar()->addMenu( "&Help" );

    // create tool bar
    QToolBar* toolbar = addToolBar( "toolbar" );

    /** FILE menu */

    // open content action
    QAction* openContentAction = new QAction( "Open Content", this );
    openContentAction->setStatusTip( "Open content" );
    connect( openContentAction, &QAction::triggered,
             this, &MasterWindow::_openContent );

    // open contents directory action
    QAction* openContentsDirectoryAction =
            new QAction( "Open Contents Directory", this );
    openContentsDirectoryAction->setStatusTip( "Open contents directory" );
    connect( openContentsDirectoryAction, &QAction::triggered,
             this, &MasterWindow::_openContentsDirectory );

    // clear contents action
    QAction* clearContentsAction = new QAction( "Clear", this );
    clearContentsAction->setStatusTip( "Clear all contents" );
    connect( clearContentsAction, &QAction::triggered,
             _displayGroup.get(), &DisplayGroup::clear );

    // save session action
    QAction* saveSessionAction = new QAction( "Save Session", this );
    saveSessionAction->setStatusTip( "Save current session" );
    connect( saveSessionAction, &QAction::triggered,
             this, &MasterWindow::_saveSession );

    // load session action
    QAction* loadSessionAction = new QAction( "Load Session", this );
    loadSessionAction->setStatusTip( "Load a session" );
    connect( loadSessionAction, &QAction::triggered,
             this, &MasterWindow::_openSession );

#if TIDE_USE_QT5WEBKITWIDGETS || TIDE_USE_QT5WEBENGINE
    // Open webbrowser action
    QAction* webbrowserAction = new QAction( "Web Browser", this );
    webbrowserAction->setStatusTip( "Open a web browser" );
    connect(webbrowserAction, &QAction::triggered,
            _webbrowserWidget, &WebbrowserWidget::show );
#endif

    // quit action
    QAction* quitAction = new QAction( "Quit", this );
    quitAction->setStatusTip( "Quit application" );
    connect(quitAction, &QAction::triggered, this, &MasterWindow::close );

    /** EDIT menu */

    // background content action
    QAction* backgroundAction = new QAction( "Background", this );
    backgroundAction->setStatusTip( "Select the background color and content" );
    connect( backgroundAction, &QAction::triggered,
             _backgroundWidget, &BackgroundWidget::show );

    /** VIEW menu */

    // enable alpha blending
    QAction* enableAlphaBlendingAction = new QAction( "Alpha Blending", this );
    enableAlphaBlendingAction->setStatusTip(
           "Enable alpha blending for transparent contents (png, svg, etc..)" );
    enableAlphaBlendingAction->setCheckable( true );
    enableAlphaBlendingAction->setChecked( _options->isAlphaBlendingEnabled( ));
    connect( enableAlphaBlendingAction, &QAction::toggled,
             _options.get(), &Options::enableAlphaBlending );
    connect( _options.get(), &Options::alphaBlendingEnabledChanged,
             enableAlphaBlendingAction, &QAction::setChecked );

    // auto focus pixel streams
    QAction* autoFocusStreamersAction =
            new QAction( "Auto-focus streamers", this );
    autoFocusStreamersAction->setStatusTip(
           "Open the windows of the external streamers in focus mode" );
    autoFocusStreamersAction->setCheckable( true );
    autoFocusStreamersAction->setChecked( _options->getAutoFocusPixelStreams());
    connect( autoFocusStreamersAction, &QAction::toggled,
             _options.get(), &Options::setAutoFocusPixelStreams );
    connect( _options.get(), &Options::autoFocusPixelStreamsChanged,
             autoFocusStreamersAction, &QAction::setChecked );

    // show clock action
    QAction* showClockAction = new QAction( "Clock", this );
    showClockAction->setStatusTip( "Show a clock on the background" );
    showClockAction->setCheckable( true );
    showClockAction->setChecked( _options->getShowClock( ));
    connect( showClockAction, &QAction::toggled, _options.get(),
             &Options::setShowClock );
    connect( _options.get(), &Options::showClockChanged,
             showClockAction, &QAction::setChecked );

    // show content tiles action
    QAction* showContentTilesAction = new QAction( "Content Tiles", this );
    showContentTilesAction->setStatusTip( "Show Content Tiles" );
    showContentTilesAction->setCheckable( true );
    showContentTilesAction->setChecked( _options->getShowContentTiles( ));
    connect( showContentTilesAction, &QAction::toggled,
             _options.get(), &Options::setShowContentTiles );
    connect( _options.get(), &Options::showContentTilesChanged,
             showContentTilesAction, &QAction::setChecked );

    // show control area action
    QAction* showControlAreaAction = new QAction( "Control Area", this );
    showControlAreaAction->setStatusTip( "Show the Control Area" );
    showControlAreaAction->setCheckable( true );
    showControlAreaAction->setChecked( _options->getShowControlArea( ));
    connect( showControlAreaAction, &QAction::toggled,
             _options.get(), &Options::setShowControlArea );
    connect( _options.get(), &Options::showControlAreaChanged,
             showControlAreaAction, &QAction::setChecked );

    // show streaming statistics action
    QAction* showStatisticsAction = new QAction( "Statistics", this );
    showStatisticsAction->setStatusTip( "Show statistics" );
    showStatisticsAction->setCheckable( true );
    showStatisticsAction->setChecked( _options->getShowStatistics( ));
    connect( showStatisticsAction, &QAction::toggled,
             _options.get(), &Options::setShowStatistics );
    connect( _options.get(), &Options::showStatisticsChanged,
             showStatisticsAction, &QAction::setChecked );

    // show test pattern action
    QAction* showTestPatternAction = new QAction( "Test Pattern", this );
    showTestPatternAction->setStatusTip( "Show test pattern" );
    showTestPatternAction->setCheckable( true );
    showTestPatternAction->setChecked( _options->getShowTestPattern( ));
    connect( showTestPatternAction, &QAction::toggled,
             _options.get(), &Options::setShowTestPattern );
    connect( _options.get(), &Options::showTestPatternChanged,
             showTestPatternAction, &QAction::setChecked );

    // show touch points action
    QAction* showTouchPoints = new QAction( "Touch Points", this );
    showTouchPoints->setStatusTip( "Show touch points" );
    showTouchPoints->setCheckable( true );
    showTouchPoints->setChecked( _options->getShowTouchPoints( ));
    connect( showTouchPoints, &QAction::toggled,
             _options.get(), &Options::setShowTouchPoints );
    connect( _options.get(), &Options::showTouchPointsChanged,
             showTouchPoints, &QAction::setChecked );

    // show window borders action
    QAction* showWindowBordersAction = new QAction( "Window Borders", this );
    showWindowBordersAction->setStatusTip( "Show window borders" );
    showWindowBordersAction->setCheckable( true );
    showWindowBordersAction->setChecked( _options->getShowWindowBorders( ));
    connect( showWindowBordersAction, &QAction::toggled,
             _options.get(), &Options::setShowWindowBorders );
    connect( _options.get(), &Options::showWindowBordersChanged,
             showWindowBordersAction, &QAction::setChecked );

    // show window title action
    QAction* showWindowTitlesAction = new QAction( "Window Titles", this );
    showWindowTitlesAction->setStatusTip( "Show window titles" );
    showWindowTitlesAction->setCheckable( true );
    showWindowTitlesAction->setChecked( _options->getShowWindowTitles( ));
    connect( showWindowTitlesAction, &QAction::toggled,
             _options.get(), &Options::setShowWindowTitles );
    connect( _options.get(), &Options::showWindowTitlesChanged,
             showWindowTitlesAction, &QAction::setChecked );

    // show zoom context action
    QAction* showZoomContextAction = new QAction( "Zoom Context", this );
    showZoomContextAction->setStatusTip( "Show zoom context" );
    showZoomContextAction->setCheckable( true );
    showZoomContextAction->setChecked( _options->getShowZoomContext( ));
    connect( showZoomContextAction, &QAction::toggled,
             _options.get(), &Options::setShowZoomContext );
    connect( _options.get(), &Options::showZoomContextChanged,
             showZoomContextAction, &QAction::setChecked );

    /** HELP menu */

    QAction* showAboutDialog = new QAction( "About", this );
    showAboutDialog->setStatusTip( "About Tide" );
    connect( showAboutDialog, &QAction::triggered,
             this, &MasterWindow::_openAboutWidget );

    // add actions to menus
    fileMenu->addAction( openContentAction );
    fileMenu->addAction( openContentsDirectoryAction );
    fileMenu->addAction( loadSessionAction );
    fileMenu->addAction( saveSessionAction );
#if TIDE_USE_QT5WEBKITWIDGETS || TIDE_USE_QT5WEBENGINE
    fileMenu->addAction( webbrowserAction );
#endif
    fileMenu->addAction( clearContentsAction );
    fileMenu->addAction( quitAction );
    editMenu->addAction( backgroundAction );
    viewMenu->addAction( autoFocusStreamersAction );
    viewMenu->addAction( enableAlphaBlendingAction );
    viewMenu->addAction( showClockAction );
    viewMenu->addAction( showContentTilesAction );
    viewMenu->addAction( showControlAreaAction );
    viewMenu->addAction( showStatisticsAction );
    viewMenu->addAction( showTestPatternAction );
    viewMenu->addAction( showTouchPoints );
    viewMenu->addAction( showWindowBordersAction );
    viewMenu->addAction( showWindowTitlesAction );
    viewMenu->addAction( showZoomContextAction );
    helpMenu->addAction( showAboutDialog );

    // add actions to toolbar
    toolbar->addAction( openContentAction );
    toolbar->addAction( openContentsDirectoryAction );
    toolbar->addAction( loadSessionAction );
    toolbar->addAction( saveSessionAction );
#if TIDE_USE_QT5WEBKITWIDGETS || TIDE_USE_QT5WEBENGINE
    toolbar->addAction( webbrowserAction );
#endif
    toolbar->addAction( clearContentsAction );
    toolbar->addAction( backgroundAction );

    // main widget / layout area
    QTabWidget* mainWidget = new QTabWidget();
    setCentralWidget( mainWidget );

    // The QWidget wrapper *has* to retain ownership to the masterQuickView.
    // It is not possible to detach the masterQuickView from its parent wrapper
    // in the MasterWindow destructor. The parentChanged event doesn't have
    // time to be processed correctly resulting in a double-delete.
    _quickView = quickView.get(); // keep a reference
    mainWidget->addTab( QWidget::createWindowContainer(
                            quickView.release( )), "Display group 0" );

    // create contents dock widget
    QDockWidget* contentsDockWidget = new QDockWidget( "Contents", this );
    QWidget* contentsWidget = new QWidget();
    QVBoxLayout* contentsLayout = new QVBoxLayout();
    contentsWidget->setLayout( contentsLayout );
    contentsDockWidget->setWidget( contentsWidget );
    addDockWidget( Qt::LeftDockWidgetArea, contentsDockWidget );

    // add the list widget
    DisplayGroupListWidget* dglwp = new DisplayGroupListWidget( this );
    dglwp->setDataModel( _displayGroup );
    contentsLayout->addWidget( dglwp );
}

void MasterWindow::_openContent()
{
    const QString filter = ContentFactory::getSupportedFilesFilterAsString();

    const QString filename = QFileDialog::getOpenFileName( this,
                                                           tr("Choose content"),
                                                           _contentFolder,
                                                           filter );
    if( filename.isEmpty( ))
        return;

    _contentFolder = QFileInfo( filename ).absoluteDir().path();

    ContentLoader loader ( _displayGroup );
    if( !loader.load( filename ))
    {
        QMessageBox messageBox;
        messageBox.setText( loader.isAlreadyOpen( filename ) ?
                                "File already open." : "Unsupported file." );
        messageBox.exec();
    }
}

void MasterWindow::_addContentDirectory( const QString& directoryName,
                                         const QSize& gridSize )
{
    QDir directory( directoryName );
    directory.setFilter( QDir::Files );
    directory.setNameFilters( ContentFactory::getSupportedFilesFilter( ));

    QFileInfoList list = directory.entryInfoList();

    // Prevent opening of folders with an excessively large number of items
    if( list.size() > 16 )
    {
        QString msg = "Opening this folder will create " +
                      QString::number( list.size( )) +
                      " content elements. Are you sure you want to continue?";

        const auto reply = QMessageBox::question( this, "Warning", msg,
                                                  QMessageBox::Yes |
                                                  QMessageBox::No );
        if( reply != QMessageBox::Yes )
            return;
    }

    ContentLoader{ _displayGroup }.loadDir( directoryName, gridSize );
}

void MasterWindow::_openContentsDirectory()
{
    const QString dirName = QFileDialog::getExistingDirectory( this, QString(),
                                                               _contentFolder );
    if( dirName.isEmpty( ))
        return;

    _contentFolder = dirName;

    const int gridX = QInputDialog::getInt( this, "Grid X dimension",
                                            "Grid X dimension", 0, 0 );
    const int gridY = QInputDialog::getInt( this, "Grid Y dimension",
                                            "Grid Y dimension", 0, 0 );
    _addContentDirectory( dirName, { gridX, gridY } );
}

void MasterWindow::_openSession()
{
    const QString filename =
            QFileDialog::getOpenFileName( this, "Load Session", _sessionFolder,
                                          SESSION_FILES_FILTER );
    if( filename.isEmpty( ))
        return;

    _sessionFolder = QFileInfo( filename ).absoluteDir().path();

    _loadSession( filename );
}

void MasterWindow::_saveSession()
{
    QString filename = QFileDialog::getSaveFileName( this, "Save Session",
                                                     _sessionFolder,
                                                     SESSION_FILES_FILTER );
    if( filename.isEmpty( ))
        return;

    _sessionFolder = QFileInfo( filename ).absoluteDir().path();

    _displayGroup->setShowWindowTitles( _options->getShowWindowTitles( ));
    _saveSessionOp.setFuture(
                StateSerializationHelper( _displayGroup ).save( filename ));
}

void MasterWindow::_loadSession( const QString& filename )
{
    _loadSessionOp.setFuture(
                StateSerializationHelper( _displayGroup ).load( filename ));
}

void MasterWindow::_openAboutWidget()
{
    const int revision = tide::Version::getRevision();

    std::ostringstream aboutMsg;
    aboutMsg << "Current version: " << tide::Version::getString();
    aboutMsg << std::endl;
    aboutMsg << "SCM revision: " << std::hex << revision << std::dec;

    QMessageBox::about( this, "About Tide", aboutMsg.str().c_str( ));
}

QStringList MasterWindow::_extractValidContentUrls( const QMimeData* mimeData )
{
    QStringList pathList;

    if( mimeData->hasUrls( ))
    {
        QList<QUrl> urlList = mimeData->urls();

        foreach( QUrl url, urlList )
        {
            const QString extension =
                    QFileInfo( url.toLocalFile().toLower( )).suffix();
            if( ContentFactory::getSupportedExtensions().contains( extension ))
                pathList.append( url.toLocalFile( ));
        }
    }

    return pathList;
}

QStringList MasterWindow::_extractFolderUrls( const QMimeData* mimeData )
{
    QStringList pathList;

    if( mimeData->hasUrls( ))
    {
        QList<QUrl> urlList = mimeData->urls();

        foreach( QUrl url, urlList )
        {
            if( QDir( url.toLocalFile( )).exists( ))
                pathList.append( url.toLocalFile( ));
        }
    }

    return pathList;
}

QString MasterWindow::_extractSessionFile( const QMimeData* mimeData )
{
    QList<QUrl> urlList = mimeData->urls();
    if( urlList.size() == 1 )
    {
        QUrl url = urlList[0];
        const QString extension =
                QFileInfo( url.toLocalFile().toLower( )).suffix();
        if( extension == "dcx" )
            return url.toLocalFile();
    }
    return QString();
}

void MasterWindow::dragEnterEvent( QDragEnterEvent* dragEvent )
{
    const QMimeData* mimeData = dragEvent->mimeData();
    const QStringList& pathList = _extractValidContentUrls( mimeData );
    const QStringList& dirList = _extractFolderUrls( mimeData );
    const QString& sessionFile = _extractSessionFile( mimeData );

    if( !pathList.empty() || !dirList.empty() || !sessionFile.isNull( ))
        dragEvent->acceptProposedAction();
}

void MasterWindow::dropEvent( QDropEvent* dropEvt )
{
    const QStringList& urls = _extractValidContentUrls( dropEvt->mimeData( ));
    ContentLoader loader( _displayGroup );
    foreach( QString url, urls )
        loader.load( url );

    const QStringList& folders = _extractFolderUrls( dropEvt->mimeData( ));
    if( !folders.isEmpty( ))
        _addContentDirectory( folders[0] ); // Only one directory at a time

    const QString& sessionFile = _extractSessionFile( dropEvt->mimeData( ));
    if( !sessionFile.isNull( ))
        _loadSession( sessionFile );

    dropEvt->acceptProposedAction();
}
