/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
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

#include "DockPixelStreamer.h"

#include "Pictureflow.h"
#include "AsyncImageLoader.h"
#include "DockToolbar.h"

#include "ContentFactory.h"
#include "thumbnail/ThumbnailGeneratorFactory.h"
#include "thumbnail/FolderThumbnailGenerator.h"

#include <deflect/Command.h>

#define DOCK_ASPECT_RATIO        0.45
#define SLIDE_REL_HEIGHT_FACTOR  0.55
#define TOOLBAR_REL_HEIGHT       0.15

#define SLIDE_MIN_SIZE           128.0
#define SLIDE_MAX_SIZE           512.0

#define COVERFLOW_SPEED_FACTOR   0.1

#define WEBBROWSER_ICON ":/img/browser-icon.png"
#define CLEARALL_ICON ":/img/clearall-icon.png"

QString DockPixelStreamer::getUniqueURI()
{
    return "Dock";
}

float DockPixelStreamer::getDefaultAspectRatio()
{
    return DOCK_ASPECT_RATIO;
}

DockPixelStreamer::DockPixelStreamer( const QSize& desiredDockSize,
                                      const QString& rootDir )
    : PixelStreamer()
    , flow_( new PictureFlow( ))
    , loader_( 0 )
    , toolbar_( 0 )
{
    const QSize& dockSize = constrainSize( desiredDockSize );
    const int toolbarHeight = dockSize.height() * TOOLBAR_REL_HEIGHT;

    createFlow( QSize( dockSize.width(), dockSize.height() - toolbarHeight ));
    createToolbar( QSize( dockSize.width(), toolbarHeight ));
    createImageLoader();

    loadThread_.start();

    if (rootDir.isEmpty() || !setRootDir(rootDir))
        setRootDir(QDir::homePath());
}

DockPixelStreamer::~DockPixelStreamer()
{
    loadThread_.quit();
    loadThread_.wait();
    delete flow_;
    delete loader_;
    delete toolbar_;
}

void DockPixelStreamer::processEvent(deflect::Event evt)
{
    if (evt.type == deflect::Event::EVT_CLICK)
    {
        processClickEvent(evt);
    }

    else if (evt.type == deflect::Event::EVT_MOVE || evt.type == deflect::Event::EVT_WHEEL)
    {
        const int offs = evt.dx * flow_->size().width() * COVERFLOW_SPEED_FACTOR;
        flow_->showSlide( flow_->centerIndex() - offs );
    }
}

void DockPixelStreamer::processClickEvent(const deflect::Event& clickEvent)
{
    // click position in pixel units inside the dock
    const int xPos = clickEvent.mouseX * flow_->size().width();
    const int yPos = clickEvent.mouseY * flow_->size().height();

    // mid is half the width of the dock in (pixel) units
    const int dockHalfWidth = flow_->size().width() / 2;

    // SlideMid is half the slide width in pixels
    const int slideHalfWidth = flow_->slideSize().width() / 2;

    // Process toolbar action
    if (yPos < (int)toolbar_->getSize().height())
    {
        const ToolbarButton* button = toolbar_->getButtonAt(QPoint(xPos,yPos));
        if (button && !button->command.isEmpty())
            emit sendCommand(button->command);
        return;
    }

    // Process flow action
    if( xPos > dockHalfWidth-slideHalfWidth && xPos < dockHalfWidth+slideHalfWidth )
    {
        onItem();
    }
    else
    {
        if( xPos > dockHalfWidth )
            flow_->showNext();
        else
            flow_->showPrevious();
    }
}

QSize DockPixelStreamer::size() const
{
    return QSize(flow_->size().width(), flow_->size().height() + toolbar_->getSize().height());
}

bool DockPixelStreamer::setRootDir(const QString& dir)
{
    if ( !QDir(dir).exists( ))
        return false;

    rootDir_ = dir;
    changeDirectory(dir);
    return true;
}

void DockPixelStreamer::onItem()
{
    const QImage& image = flow_->slide( flow_->centerIndex( ));
    const QString& source = image.text( "source" );

    if( image.text( "dir" ).isEmpty( ))
    {
        deflect::Command command(deflect::COMMAND_TYPE_FILE, source);
        emit sendCommand(command.getCommand());
    }
    else
    {
        changeDirectory( source );
    }
}

void DockPixelStreamer::update(const QImage& image)
{
    QImage newImage(size(), image.format());

    // Copy Toolbar
    uchar* dst = newImage.bits();
    const QImage& toolbarImage = toolbar_->getImage();
    memcpy(dst, toolbarImage.bits(), toolbarImage.byteCount());
    dst += toolbarImage.byteCount();

    // Copy Flow
    memcpy(dst, image.bits(), image.byteCount());

    emit imageUpdated(newImage);
}

void DockPixelStreamer::loadThumbnails(int newCenterIndex)
{
    const int nbThumbnails = 2;

    slideImagesToLoad_.clear();

    int imin = std::max(newCenterIndex-nbThumbnails, 0);
    int imax = std::min(newCenterIndex+nbThumbnails, slideImagesLoaded_.size()-1);
    for (int i=imin; i<=imax; i++)
    {
        slideImagesToLoad_.append(i);
    }
    loadNextThumbnailInList();
}

void DockPixelStreamer::loadNextThumbnailInList()
{
    while(!slideImagesToLoad_.empty())
    {
        int i = slideImagesToLoad_.front();
        slideImagesToLoad_.pop_front();
        if (!slideImagesLoaded_[i].first)
        {
            slideImagesLoaded_[i].first = true;
            emit renderPreview( slideImagesLoaded_[i].second, i );
            return;
        }
    }
}

void DockPixelStreamer::createFlow(const QSize& dockSize)
{
    const unsigned int slideSize = dockSize.height() * SLIDE_REL_HEIGHT_FACTOR;
    flow_->resize(dockSize);
    flow_->setSlideSize( QSize( slideSize, slideSize ));
    flow_->setBackgroundColor( Qt::darkGray );

    connect( flow_, SIGNAL( imageUpdated( const QImage& )), this, SLOT( update( const QImage& )));
    connect( flow_, SIGNAL( targetIndexChanged(int)), this, SLOT(loadThumbnails(int)) );
}

void DockPixelStreamer::createToolbar( const QSize& toolbarSize )
{
    toolbar_ = new DockToolbar( toolbarSize );

    deflect::Command webbrowserCommand( deflect::COMMAND_TYPE_WEBBROWSER, "" );
    toolbar_->addButton( new ToolbarButton( "Webbrowser",
                                            QImage( WEBBROWSER_ICON ),
                                            webbrowserCommand.getCommand( )));

    deflect::Command clearallCommand( deflect::COMMAND_TYPE_SESSION,
                                      "clearall" );
    toolbar_->addButton( new ToolbarButton( "Clear all",
                                            QImage( CLEARALL_ICON ),
                                            clearallCommand.getCommand( )));
}

void DockPixelStreamer::createImageLoader()
{
    loader_ = new AsyncImageLoader(flow_->slideSize());
    loader_->moveToThread( &loadThread_ );
    connect( loader_, SIGNAL(imageLoaded(int, QImage)),
             flow_, SLOT(setSlide( int, QImage )));
    connect( this, SIGNAL(renderPreview( const QString&, const int )),
             loader_, SLOT(loadImage( const QString&, const int )));
    connect( loader_, SIGNAL(imageLoadingFinished()),
             this, SLOT(loadNextThumbnailInList()));
}

void DockPixelStreamer::changeDirectory( const QString& dir )
{
    slideIndex_[currentDir_.path()] = flow_->centerIndex();

    flow_->clear();
    slideImagesToLoad_.clear();
    slideImagesLoaded_.clear();

    currentDir_ = QDir(dir);
    if (dir != rootDir_)
    {
        addRootDirToFlow();
    }
    addFilesToFlow();
    addFoldersToFlow();

    flow_->setCenterIndex( slideIndex_[currentDir_.path()] );
    loadThumbnails( slideIndex_[currentDir_.path()] );
}

void DockPixelStreamer::addRootDirToFlow()
{
    QDir rootDir = currentDir_;
    const bool hasRootDir = rootDir.cdUp();

    if( hasRootDir)
    {
        FolderThumbnailGeneratorPtr folderGenerator = ThumbnailGeneratorFactory::getFolderGenerator(flow_->slideSize());

        QImage img = folderGenerator->generateUpFolderImage(rootDir);
        flow_->addSlide( img, "UP: " + rootDir.path() );
        slideImagesLoaded_.append(qMakePair(true, QString()));
    }
}

void DockPixelStreamer::addFilesToFlow()
{
    currentDir_.setFilter( QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot );
    QStringList filters = ContentFactory::getSupportedFilesFilter();
    filters.append( "*.dcx" );
    currentDir_.setNameFilters( filters );
    const QFileInfoList& fileList = currentDir_.entryInfoList();

    ThumbnailGeneratorPtr defaultGenerator = ThumbnailGeneratorFactory::getDefaultGenerator(flow_->slideSize());

    for( int i = 0; i < fileList.size(); ++i )
    {
        const QFileInfo& fileInfo = fileList.at( i );
        const QString& fileName = currentDir_.absoluteFilePath( fileInfo.fileName( ));
        QImage img = defaultGenerator->generate(fileName);
        flow_->addSlide( img, fileInfo.fileName( ));
        slideImagesLoaded_.append(qMakePair(false, fileName));
    }
}

void DockPixelStreamer::addFoldersToFlow()
{
    currentDir_.setFilter( QDir::Dirs | QDir::NoDotAndDotDot );
    currentDir_.setNameFilters( QStringList( ));
    const QFileInfoList& dirList = currentDir_.entryInfoList();

    FolderThumbnailGeneratorPtr folderGenerator = ThumbnailGeneratorFactory::getFolderGenerator(flow_->slideSize());

    for( int i = 0; i < dirList.size(); ++i )
    {
        const QFileInfo& fileInfo = dirList.at( i );
        const QString& fileName = currentDir_.absoluteFilePath( fileInfo.fileName( ));
        if( !fileName.endsWith( ".pyramid" ))
        {
            QImage img = folderGenerator->generatePlaceholderImage(QDir(fileName));
            flow_->addSlide( img, fileInfo.fileName( ));
            slideImagesLoaded_.append(qMakePair(false, fileName));
        }
    }
}

QSize DockPixelStreamer::getMinSize()
{
    const qreal dockHeight = SLIDE_MIN_SIZE / SLIDE_REL_HEIGHT_FACTOR;
    const qreal dockWidth = dockHeight / getDefaultAspectRatio();
    return QSize( dockWidth, dockHeight );
}

QSize DockPixelStreamer::getMaxSize()
{
    const qreal dockHeight = SLIDE_MAX_SIZE / SLIDE_REL_HEIGHT_FACTOR;
    const qreal dockWidth = dockHeight / getDefaultAspectRatio();
    return QSize( dockWidth, dockHeight );
}

QSize DockPixelStreamer::constrainSize( const QSize& targetSize )
{
    const QSize minSize = getMinSize();
    const QSize maxSize = getMaxSize();

    if( targetSize < minSize )
        return minSize;

    if( targetSize > maxSize )
        return maxSize;

    return targetSize;
}
