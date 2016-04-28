/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
/* Copyright (c) 2013-2015, EPFL/Blue Brain Project                  */
/*                     Raphael.Dumusc@epfl.ch                        */
/*                     Daniel.Nachbaur@epfl.ch                       */
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

#include "ContentWindow.h"

#include "ContentInteractionDelegate.h"
#include "ContentWindowController.h"

#include "config.h"
#include "log.h"

#include "PixelStreamInteractionDelegate.h"
#include "ZoomInteractionDelegate.h"
#if TIDE_ENABLE_PDF_SUPPORT
#  include "PDFInteractionDelegate.h"
#endif

IMPLEMENT_SERIALIZE_FOR_XML( ContentWindow )

ContentWindow::ContentWindow( ContentPtr content, const WindowType type )
    : uuid_( QUuid::createUuid( ))
    , type_( type )
    , content_( content )
    , controller_( 0 )
    , windowBorder_( NOBORDER )
    , focused_( false )
    , windowState_( NONE )
    , controlsVisible_( false )
{
    assert( content );
    init();
    coordinates_.setSize( content_->getDimensions( ));
}

ContentWindow::ContentWindow()
    : uuid_( QUuid::createUuid( ))
    , type_( WindowType::DEFAULT )
    , controller_( 0 )
    , windowBorder_( NOBORDER )
    , focused_( false )
    , windowState_( NONE )
    , controlsVisible_( false )
{}

ContentWindow::~ContentWindow()
{
}

const QUuid& ContentWindow::getID() const
{
    return uuid_;
}

bool ContentWindow::isPanel() const
{
    return type_ == WindowType::PANEL;
}

Content* ContentWindow::getContentPtr() const
{
    return content_.get();
}

ContentPtr ContentWindow::getContent() const
{
    return content_;
}

void ContentWindow::setContent( ContentPtr content )
{
    assert( content );

    if( content_ )
        content_->disconnect( this, SIGNAL( contentModified( )));

    content_ = content;
    init();
}

ContentWindowController* ContentWindow::getController()
{
    return controller_;
}

const ContentWindowController* ContentWindow::getController() const
{
    return controller_;
}

void ContentWindow::setController( ContentWindowControllerPtr controller )
{
    controller_ = controller.release();
    controller_->setParent( this );
}

void ContentWindow::setCoordinates( const QRectF& coordinates )
{
    if( coordinates == coordinates_ )
        return;

    setX( coordinates.x( ));
    setY( coordinates.y( ));
    setWidth( coordinates.width( ));
    setHeight( coordinates.height( ));

    emit coordinatesChanged();

    emit modified();
}

ContentWindow::WindowBorder ContentWindow::getBorder() const
{
    return windowBorder_;
}

ContentWindow::WindowState ContentWindow::getState() const
{
    return windowState_;
}

void ContentWindow::setBorder( const ContentWindow::WindowBorder border )
{
    if( windowBorder_ == border )
        return;
    windowBorder_ = border;
    emit borderChanged();
    emit modified();
}

bool ContentWindow::isFocused() const
{
    return focused_;
}

const QRectF& ContentWindow::getFocusedCoordinates() const
{
    return focusedCoordinates_;
}

void ContentWindow::setFocusedCoordinates( const QRectF& coordinates )
{
    if( coordinates == focusedCoordinates_ )
        return;

    focusedCoordinates_ = coordinates;
    emit focusedCoordinatesChanged();
}

const QRectF& ContentWindow::getDisplayCoordinates() const
{
    return isFocused() ? getFocusedCoordinates() : getCoordinates();
}

void ContentWindow::setFocused( const bool value )
{
    if( focused_ == value )
        return;

    focused_ = value;

    emit focusedChanged();

    // Only emit modified once, in setState() or here
    if( !setState( focused_ ? SELECTED : NONE ))
        emit modified();
}

bool ContentWindow::setState( const ContentWindow::WindowState state )
{
    if( windowState_ == state )
        return false;

    windowState_ = state;

    emit stateChanged();
    emit modified();
    return true;
}

void ContentWindow::toggleSelectedState()
{
    if ( windowState_ == ContentWindow::NONE )
        setState( ContentWindow::SELECTED );
    else if ( windowState_ == ContentWindow::SELECTED )
        setState( ContentWindow::NONE );
}

bool ContentWindow::isSelected() const
{
    return windowState_ == SELECTED;
}

bool ContentWindow::isMoving() const
{
    return windowState_ == MOVING;
}

bool ContentWindow::isResizing() const
{
    return windowState_ == RESIZING;
}

bool ContentWindow::isHidden() const
{
    return windowState_ == HIDDEN;
}

ContentInteractionDelegate* ContentWindow::getInteractionDelegate()
{
    return interactionDelegate_.get();
}

QString ContentWindow::getLabel() const
{
    return content_->getURI().section( "/", -1, -1 );
}

bool ContentWindow::getControlsVisible() const
{
    return controlsVisible_;
}

void ContentWindow::setControlsVisible( const bool value )
{
    if( value == controlsVisible_ )
        return;

    controlsVisible_ = value;
    emit controlsVisibleChanged();
    emit modified();
}

void ContentWindow::init()
{
    connect( content_.get(), SIGNAL( modified( )), SIGNAL( contentModified( )));
    createInteractionDelegate();
}

void ContentWindow::createInteractionDelegate()
{
    assert( content_ );

    switch ( content_->getType( ))
    {
    case CONTENT_TYPE_PIXEL_STREAM:
        interactionDelegate_.reset( new PixelStreamInteractionDelegate( *this ));
        break;
    case CONTENT_TYPE_MOVIE:
        interactionDelegate_.reset( new ContentInteractionDelegate( *this ));
        break;
#if TIDE_ENABLE_PDF_SUPPORT
    case CONTENT_TYPE_PDF:
        interactionDelegate_.reset( new PDFInteractionDelegate( *this ));
        break;
#endif
    default:
        interactionDelegate_.reset( new ZoomInteractionDelegate( *this ));
        break;
    }
}
