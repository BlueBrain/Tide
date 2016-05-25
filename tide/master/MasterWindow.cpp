/*********************************************************************/
/* Copyright (c) 2014-2016, EPFL/Blue Brain Project                  */
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

#include "log.h"
#include "Options.h"
#include "MasterConfiguration.h"

#include "ContentLoader.h"
#include "ContentFactory.h"
#include "StateSerializationHelper.h"

#include "DynamicTexture.h"

#include "DisplayGroup.h"
#include "ContentWindow.h"

#include "DisplayGroupView.h"
#include "DisplayGroupListWidget.h"
#include "BackgroundWidget.h"
#ifdef TIDE_USE_QT5WEBKITWIDGETS
#  include "WebbrowserWidget.h"
#endif

#include <tide/core/version.h>

#include <QtWidgets>
#include <sstream>

namespace
{
const QString STATE_FILES_FILTER( "State files (*.dcx)" );
const QSize DEFAULT_WINDOW_SIZE( 800, 600 );
}

MasterWindow::MasterWindow( DisplayGroupPtr displayGroup,
                            MasterConfiguration& config )
    : QMainWindow()
    , _displayGroup( displayGroup )
    , _options( new Options )
    , _backgroundWidget( new BackgroundWidget( config, this ))
#ifdef TIDE_USE_QT5WEBKITWIDGETS
    , _webbrowserWidget( new WebbrowserWidget( config, this ))
#endif
    , _displayGroupView( new DisplayGroupView( _options, config ))
    , _autoFocusPixelStreamsAction( 0 )
    , _contentFolder( config.getContentDir( ))
    , _sessionFolder( config.getSessionsDir( ))
{
    _backgroundWidget->setModal( true );

    connect( _backgroundWidget, SIGNAL( backgroundColorChanged( QColor )),
             _options.get(), SLOT( setBackgroundColor( QColor )));
    connect( _backgroundWidget, SIGNAL( backgroundContentChanged( ContentPtr )),
             _options.get(), SLOT( setBackgroundContent( ContentPtr )));

#ifdef TIDE_USE_QT5WEBKITWIDGETS
    connect( _webbrowserWidget,
             SIGNAL( openWebBrowser( QPointF, QSize, QString )),
             this, SIGNAL( openWebBrowser( QPointF, QSize, QString )));
#endif

    resize( DEFAULT_WINDOW_SIZE );
    setAcceptDrops( true );

    _setupMasterWindowUI();

    show();
}

MasterWindow::~MasterWindow() {}

DisplayGroupView* MasterWindow::getDisplayGroupView()
{
    return _displayGroupView;
}

OptionsPtr MasterWindow::getOptions() const
{
    return _options;
}

QAction* MasterWindow::getAutoFocusPixelStreamsAction()
{
    return _autoFocusPixelStreamsAction;
}

void MasterWindow::_setupMasterWindowUI()
{
    // create menus in menu bar
    QMenu* fileMenu = menuBar()->addMenu("&File");
    QMenu* editMenu = menuBar()->addMenu("&Edit");
    QMenu* viewMenu = menuBar()->addMenu("&View");
    QMenu* toolsMenu = menuBar()->addMenu("&Tools");
    QMenu* helpMenu = menuBar()->addMenu("&Help");

    // create tool bar
    QToolBar* toolbar = addToolBar("toolbar");

    // open content action
    QAction* openContentAction = new QAction("Open Content", this);
    openContentAction->setStatusTip("Open content");
    connect(openContentAction, SIGNAL(triggered()), this, SLOT(openContent()));

    // open contents directory action
    QAction* openContentsDirectoryAction = new QAction("Open Contents Directory", this);
    openContentsDirectoryAction->setStatusTip("Open contents directory");
    connect(openContentsDirectoryAction, SIGNAL(triggered()), this, SLOT(openContentsDirectory()));

    // clear contents action
    QAction* clearContentsAction = new QAction("Clear", this);
    clearContentsAction->setStatusTip("Clear");
    connect(clearContentsAction, SIGNAL(triggered()), _displayGroup.get(), SLOT(clear()));

    // save state action
    QAction* saveStateAction = new QAction("Save State", this);
    saveStateAction->setStatusTip("Save state");
    connect(saveStateAction, SIGNAL(triggered()), this, SLOT(saveState()));

    // load state action
    QAction* loadStateAction = new QAction("Load State", this);
    loadStateAction->setStatusTip("Load state");
    connect(loadStateAction, SIGNAL(triggered()), this, SLOT(loadState()));

    // compute image pyramid action
    QAction* computeImagePyramidAction = new QAction("Compute Image Pyramid", this);
    computeImagePyramidAction->setStatusTip("Compute image pyramid");
    connect(computeImagePyramidAction, SIGNAL(triggered()), this, SLOT(computeImagePyramid()));

    // load background content action
    QAction* backgroundAction = new QAction("Background", this);
    backgroundAction->setStatusTip("Select the background color and content");
    connect(backgroundAction, SIGNAL(triggered()), _backgroundWidget, SLOT(show()));

#ifdef TIDE_USE_QT5WEBKITWIDGETS
    // Open webbrowser action
    QAction* webbrowserAction = new QAction("Web Browser", this);
    webbrowserAction->setStatusTip("Open a web browser");
    connect(webbrowserAction, SIGNAL(triggered()), _webbrowserWidget, SLOT(show()));
#endif

    // quit action
    QAction* quitAction = new QAction("Quit", this);
    quitAction->setStatusTip("Quit application");
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    // show clock action
    QAction* showClockAction = new QAction( "Show Clock", this );
    showClockAction->setStatusTip( "Show a clock on the background" );
    showClockAction->setCheckable( true );
    showClockAction->setChecked( _options->getShowClock( ));
    connect( showClockAction, &QAction::toggled, _options.get(),
             &Options::setShowClock );
    connect( _options.get(), &Options::showClockChanged,
             showClockAction, &QAction::setChecked );

    // show window borders action
    QAction* showWindowBordersAction = new QAction("Show Window Borders", this);
    showWindowBordersAction->setStatusTip("Show window borders");
    showWindowBordersAction->setCheckable(true);
    showWindowBordersAction->setChecked(_options->getShowWindowBorders());
    connect(showWindowBordersAction, SIGNAL(toggled(bool)), _options.get(), SLOT(setShowWindowBorders(bool)));

    // show touch points action
    QAction* showTouchPoints = new QAction("Show Touch Points", this);
    showTouchPoints->setStatusTip("Show touch points");
    showTouchPoints->setCheckable(true);
    showTouchPoints->setChecked(_options->getShowTouchPoints());
    connect(showTouchPoints, SIGNAL(toggled(bool)), _options.get(), SLOT(setShowTouchPoints(bool)));

    // show test pattern action
    QAction* showTestPatternAction = new QAction("Show Test Pattern", this);
    showTestPatternAction->setStatusTip("Show test pattern");
    showTestPatternAction->setCheckable(true);
    showTestPatternAction->setChecked(_options->getShowTestPattern());
    connect(showTestPatternAction, SIGNAL(toggled(bool)), _options.get(), SLOT(setShowTestPattern(bool)));

    // show zoom context action
    QAction* showZoomContextAction = new QAction("Show Zoom Context", this);
    showZoomContextAction->setStatusTip("Show zoom context");
    showZoomContextAction->setCheckable(true);
    showZoomContextAction->setChecked(_options->getShowZoomContext());
    connect(showZoomContextAction, SIGNAL(toggled(bool)), _options.get(), SLOT(setShowZoomContext(bool)));

    // show content tiles action
    QAction* showContentTilesAction = new QAction("Show Content Tiles", this);
    showContentTilesAction->setStatusTip("Show Content Tiles");
    showContentTilesAction->setCheckable(true);
    showContentTilesAction->setChecked(_options->getShowContentTiles());
    connect(showContentTilesAction, SIGNAL(toggled(bool)), _options.get(), SLOT(setShowContentTiles(bool)));

    // show streaming statistics action
    QAction* showStatisticsAction = new QAction("Show Statistics", this);
    showStatisticsAction->setStatusTip("Show statistics");
    showStatisticsAction->setCheckable(true);
    showStatisticsAction->setChecked(_options->getShowStatistics());
    connect(showStatisticsAction, SIGNAL(toggled(bool)), _options.get(), SLOT(setShowStatistics(bool)));

    // show window title action
    QAction* showWindowTitlesAction = new QAction( "Show Window Titles", this );
    showWindowTitlesAction->setStatusTip( "Show window titles" );
    showWindowTitlesAction->setCheckable( true );
    showWindowTitlesAction->setChecked( _displayGroup->getShowWindowTitles( ));
    connect( showWindowTitlesAction, SIGNAL( toggled( bool )),
             _displayGroup.get(), SLOT( setShowWindowTitles( bool )));
    connect( _displayGroup.get(), SIGNAL( showWindowTitlesChanged( bool )),
             showWindowTitlesAction, SLOT( setChecked( bool )));

    // show control area action
    QAction* showControlAreaAction = new QAction( "Show Control Area", this );
    showControlAreaAction->setStatusTip( "Show the Control Area" );
    showControlAreaAction->setCheckable( true );
    showControlAreaAction->setChecked( _options->getShowControlArea( ));
    connect( showControlAreaAction, &QAction::toggled,
             _options.get(), &Options::setShowControlArea );

    // enable alpha blending
    QAction* enableAlphaBlendingAction = new QAction( "Alpha Blending", this );
    enableAlphaBlendingAction->setStatusTip(
           "Enable alpha blending for transparent contents (png, svg, etc..)" );
    enableAlphaBlendingAction->setCheckable( true );
    enableAlphaBlendingAction->setChecked( _options->isAlphaBlendingEnabled( ));
    connect( enableAlphaBlendingAction, SIGNAL( toggled( bool )),
             _options.get(), SLOT( enableAlphaBlending( bool )));

    // auto focus pixel streams
    _autoFocusPixelStreamsAction = new QAction( "Auto-focus streamers", this );
    _autoFocusPixelStreamsAction->setStatusTip(
           "Open the windows of the external streamers in focus mode" );
    _autoFocusPixelStreamsAction->setCheckable( true );

    QAction* showAboutDialog = new QAction("About", this);
    showAboutDialog->setStatusTip("About Tide");
    connect(showAboutDialog, SIGNAL(triggered()), this, SLOT(openAboutWidget()));

    // add actions to menus
    fileMenu->addAction( openContentAction );
    fileMenu->addAction( openContentsDirectoryAction );
    fileMenu->addAction( loadStateAction );
    fileMenu->addAction( saveStateAction );
#ifdef TIDE_USE_QT5WEBKITWIDGETS
    fileMenu->addAction( webbrowserAction );
#endif
    fileMenu->addAction( clearContentsAction );
    fileMenu->addAction( quitAction );
    editMenu->addAction( backgroundAction );
    viewMenu->addAction( showClockAction );
    viewMenu->addAction( showStatisticsAction );
    viewMenu->addAction( showWindowTitlesAction );
    viewMenu->addAction( showContentTilesAction );
    viewMenu->addAction( showWindowBordersAction );
    viewMenu->addAction( showTouchPoints );
    viewMenu->addAction( showTestPatternAction );
    viewMenu->addAction( showZoomContextAction );
    viewMenu->addAction( showControlAreaAction );
    viewMenu->addAction( enableAlphaBlendingAction );
    viewMenu->addAction( _autoFocusPixelStreamsAction );
    toolsMenu->addAction( computeImagePyramidAction );

    helpMenu->addAction( showAboutDialog );

    // add actions to toolbar
    toolbar->addAction(openContentAction);
    toolbar->addAction(openContentsDirectoryAction);
    toolbar->addAction(loadStateAction);
    toolbar->addAction(saveStateAction);
#ifdef TIDE_USE_QT5WEBKITWIDGETS
    toolbar->addAction(webbrowserAction);
#endif
    toolbar->addAction(clearContentsAction);
    toolbar->addAction(backgroundAction);

    // main widget / layout area
    QTabWidget* mainWidget = new QTabWidget();
    setCentralWidget(mainWidget);

    // add the local renderer group
    _displayGroupView->setDataModel( _displayGroup );
    QWidget* wrapper = QWidget::createWindowContainer( _displayGroupView,
                                                       mainWidget );
    mainWidget->addTab( wrapper, "Display group 0" );

    // Forward background touch events
    connect( _displayGroupView, &DisplayGroupView::backgroundTap,
             this, &MasterWindow::hideLauncher );
    connect( _displayGroupView, &DisplayGroupView::backgroundTapAndHold,
             this, &MasterWindow::openLauncher );

    // Forward controls touch events
    connect( _displayGroupView, &DisplayGroupView::launcherControlPressed,
             [this]()
    {
        for( auto win : _displayGroup->getContentWindows( ))
        {
            if( win->isPanel() && !win->isHidden( ))
            {
                emit hideLauncher();
                return;
            }
        }
        emit openLauncher();
    });
    connect( _displayGroupView, &DisplayGroupView::settingsControlsPressed,
             [this]()
    {
        _options->setShowClock( !_options->getShowClock( ));
    });

    // create contents dock widget
    QDockWidget* contentsDockWidget = new QDockWidget("Contents", this);
    QWidget* contentsWidget = new QWidget();
    QVBoxLayout* contentsLayout = new QVBoxLayout();
    contentsWidget->setLayout(contentsLayout);
    contentsDockWidget->setWidget(contentsWidget);
    addDockWidget(Qt::LeftDockWidgetArea, contentsDockWidget);

    // add the list widget
    DisplayGroupListWidget* dglwp = new DisplayGroupListWidget(this);
    dglwp->setDataModel(_displayGroup);
    contentsLayout->addWidget(dglwp);
}

void MasterWindow::openContent()
{
    const QString filter = ContentFactory::getSupportedFilesFilterAsString();

    const QString filename = QFileDialog::getOpenFileName( this,
                                                           tr("Choose content"),
                                                           _contentFolder,
                                                           filter );
    if( filename.isEmpty( ))
        return;

    _contentFolder = QFileInfo( filename ).absoluteDir().path();

    if( !ContentLoader( _displayGroup ).load( filename ))
    {
        QMessageBox messageBox;
        messageBox.setText( "Unsupported file." );
        messageBox.exec();
    }
}

void MasterWindow::_addContentDirectory( const QString& directoryName,
                                        unsigned int gridX,
                                        unsigned int gridY )
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

        typedef QMessageBox::StandardButton button;
        const button reply = QMessageBox::question( this, "Warning", msg,
                                                    QMessageBox::Yes |
                                                    QMessageBox::No );
        if ( reply != QMessageBox::Yes )
            return;
    }

    // If the grid size is unspecified, compute one large enough to hold all the elements
    if ( gridX == 0 || gridY == 0 )
        _estimateGridSize( list.size(), gridX, gridY );

    unsigned int contentIndex = 0;

    const QSizeF win( _displayGroup->getCoordinates().width() / (qreal)gridX,
                      _displayGroup->getCoordinates().height() / (qreal)gridY );

    ContentLoader contentLoader( _displayGroup );

    for( int i = 0; i < list.size() && contentIndex < gridX * gridY; ++i )
    {
        const QFileInfo& fileInfo = list.at(i);
        const QString& filename = fileInfo.absoluteFilePath();

        const unsigned int x_coord = contentIndex % gridX;
        const unsigned int y_coord = contentIndex / gridX;
        const QPointF position( x_coord * win.width() + 0.5 * win.width(),
                                y_coord * win.height() + 0.5 * win.height( ));

        if( contentLoader.load( filename, position, win ))
            ++contentIndex;
    }
}

void MasterWindow::openContentsDirectory()
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
    assert( gridX >= 0 && gridY >= 0 );

    _addContentDirectory( dirName, gridX, gridY );
}

void MasterWindow::openAboutWidget()
{
    const int revision = tide::Version::getRevision();

    std::ostringstream aboutMsg;
    aboutMsg << "Current version: " << tide::Version::getString();
    aboutMsg << std::endl;
    aboutMsg << "SCM revision: " << std::hex << revision << std::dec;

    QMessageBox::about( this, "About Tide", aboutMsg.str().c_str( ));
}

void MasterWindow::saveState()
{
    QString filename = QFileDialog::getSaveFileName( this, "Save State",
                                                     _sessionFolder,
                                                     STATE_FILES_FILTER );
    if( filename.isEmpty( ))
        return;

    _sessionFolder = QFileInfo( filename ).absoluteDir().path();

    // make sure filename has .dcx extension
    if( !filename.endsWith( ".dcx" ))
    {
        put_flog( LOG_VERBOSE, "appended .dcx filename extension" );
        filename.append( ".dcx" );
    }

    if( !StateSerializationHelper( _displayGroup ).save( filename ))
    {
        QMessageBox::warning( this, "Error", "Could not save state file.",
                              QMessageBox::Ok, QMessageBox::Ok );
    }
}

void MasterWindow::loadState()
{
    const QString filename = QFileDialog::getOpenFileName( this, "Load State",
                                                           _sessionFolder,
                                                           STATE_FILES_FILTER );
    if( filename.isEmpty( ))
        return;

    _sessionFolder = QFileInfo( filename ).absoluteDir().path();

    _loadState( filename );
}

void MasterWindow::_loadState( const QString& filename )
{
    if( !StateSerializationHelper( _displayGroup ).load( filename ))
    {
        QMessageBox::warning( this, "Error", "Could not load state file.",
                              QMessageBox::Ok, QMessageBox::Ok );
    }
}

void MasterWindow::computeImagePyramid()
{
    const QString filename = QFileDialog::getOpenFileName( this, "Select image",
                                                           _contentFolder );
    if( filename.isEmpty( ))
        return;

    _contentFolder = QFileInfo( filename ).absoluteDir().path();

    put_flog( LOG_DEBUG, "source image filename: %s",
              filename.toLocal8Bit().constData( ));

    put_flog( LOG_DEBUG, "target location for image pyramid folder: %s",
              _contentFolder.toLocal8Bit().constData( ));

    DynamicTexturePtr dynamicTexture( new DynamicTexture( filename ));
    if( !dynamicTexture->generateImagePyramid( _contentFolder ))
    {
        QMessageBox::warning( this, "Error", "Image pyramid creation failed.",
                              QMessageBox::Ok, QMessageBox::Ok );
    }

    put_flog( LOG_DEBUG, "done generating pyramid" );
}

void MasterWindow::_estimateGridSize( unsigned int numElem, unsigned int &gridX,
                                     unsigned int &gridY )
{
    assert( numElem > 0 );
    gridX = (unsigned int)( ceil( sqrt( numElem )));
    assert( gridX > 0 );
    gridY = ( gridX * ( gridX-1 ) >= numElem ) ? gridX-1 : gridX;
}

QStringList MasterWindow::_extractValidContentUrls(const QMimeData* mimeData)
{
    QStringList pathList;

    if (mimeData->hasUrls())
    {
        QList<QUrl> urlList = mimeData->urls();

        foreach (QUrl url, urlList)
        {
            QString extension = QFileInfo(url.toLocalFile().toLower()).suffix();
            if (ContentFactory::getSupportedExtensions().contains(extension))
                pathList.append(url.toLocalFile());
        }
    }

    return pathList;
}

QStringList MasterWindow::_extractFolderUrls(const QMimeData* mimeData)
{
    QStringList pathList;

    if (mimeData->hasUrls())
    {
        QList<QUrl> urlList = mimeData->urls();

        foreach (QUrl url, urlList)
        {
            if (QDir(url.toLocalFile()).exists())
                pathList.append(url.toLocalFile());
        }
    }

    return pathList;
}

QString MasterWindow::_extractStateFile(const QMimeData* mimeData)
{
    QList<QUrl> urlList = mimeData->urls();
    if (urlList.size() == 1)
    {
        QUrl url = urlList[0];
        QString extension = QFileInfo(url.toLocalFile().toLower()).suffix();
        if (extension == "dcx")
            return url.toLocalFile();
    }
    return QString();
}

void MasterWindow::dragEnterEvent(QDragEnterEvent* dragEvent)
{
    const QStringList& pathList = _extractValidContentUrls(dragEvent->mimeData());
    const QStringList& dirList = _extractFolderUrls(dragEvent->mimeData());
    const QString& stateFile = _extractStateFile(dragEvent->mimeData());

    if (!pathList.empty() || !dirList.empty() || !stateFile.isNull())
    {
        dragEvent->acceptProposedAction();
    }
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

    const QString& stateFile = _extractStateFile( dropEvt->mimeData( ));
    if( !stateFile.isNull( ))
        _loadState( stateFile );

    dropEvt->acceptProposedAction();
}
