/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
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

#include "DisplayGroupListWidget.h"

#include "ContentWindowListWidgetItem.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"

#include <QListWidget>

DisplayGroupListWidget::DisplayGroupListWidget( QWidget* parent_ )
    : QListWidget( parent_ )
{
    connect( this, &QListWidget::itemClicked,
             this, &DisplayGroupListWidget::_onItemClicked );
}

void DisplayGroupListWidget::setDataModel( DisplayGroupPtr displayGroup )
{
    if( _displayGroup )
        _displayGroup->disconnect( this );
    clear();

    _displayGroup = displayGroup;
    if( !_displayGroup )
        return;

    connect( _displayGroup.get(), &DisplayGroup::contentWindowAdded,
             this, &DisplayGroupListWidget::_addContentWindow );
    connect( _displayGroup.get(), &DisplayGroup::contentWindowRemoved,
             this, &DisplayGroupListWidget::_removeContentWindow );
    connect( _displayGroup.get(), &DisplayGroup::contentWindowMovedToFront,
             this, &DisplayGroupListWidget::_moveToFront );
}

void DisplayGroupListWidget::_addContentWindow( ContentWindowPtr contentWindow )
{
    auto newItem = new ContentWindowListWidgetItem( contentWindow );
    newItem->setText( contentWindow->getContent()->getTitle( ));
    connect( contentWindow->getContentPtr(), &Content::titleChanged,
             [newItem]( const QString title ) { newItem->setText( title ); });
    insertItem( 0, newItem );
}

void DisplayGroupListWidget::_removeContentWindow( ContentWindowPtr
                                                   contentWindow )
{
    for( int i = 0; i < count(); ++i )
    {
        auto listWidgetItem = item( i );
        auto windowItem =
                dynamic_cast< ContentWindowListWidgetItem* >( listWidgetItem );
        if( windowItem && windowItem->getContentWindow() == contentWindow )
        {
            takeItem( i );
            delete listWidgetItem;
            return;
        }
    }
}

void DisplayGroupListWidget::_moveToFront( ContentWindowPtr contentWindow )
{
    for( int i = 0; i < count(); ++i )
    {
        auto listWidgetItem = item( i );
        auto windowItem =
                dynamic_cast< ContentWindowListWidgetItem* >( listWidgetItem );
        if( windowItem && windowItem->getContentWindow() == contentWindow )
        {
            takeItem( i );
            insertItem( 0, listWidgetItem );
            return;
        }
    }
}

void DisplayGroupListWidget::_onItemClicked( QListWidgetItem* item_ )
{
    if( auto windowItem = dynamic_cast< ContentWindowListWidgetItem* >( item_ ))
        _displayGroup->moveToFront( windowItem->getContentWindow( ));
}
