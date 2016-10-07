/*********************************************************************/
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
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

#include "StateSerializationHelper.h"

#include "control/DisplayGroupController.h"
#include "log.h"
#include "scene/ContentFactory.h"
#include "scene/DisplayGroup.h"
#include "serialization/utils.h"
#include "State.h"
#include "StatePreview.h"

namespace
{
const QString SESSION_FILE_EXTENSION( ".dcx" );
}

StateSerializationHelper::StateSerializationHelper( DisplayGroupPtr group )
    : _displayGroup( group )
{
}

bool _canBeRestored( const CONTENT_TYPE type )
{
    // PixelStreams are external applications and can't be restored.
    if( type == CONTENT_TYPE_PIXEL_STREAM )
        return false;

    // Webbrowsers are not supported yet, don't restore them.
    if( type == CONTENT_TYPE_WEBBROWSER )
        return false;

    return true;
}

bool _validateContent( const ContentWindowPtr& window )
{
    ContentPtr content = window->getContent();
    if( !content )
    {
        put_flog( LOG_WARN, "Window '%s' does not have a Content!",
            window->getID().toString().toLocal8Bit().constData( ));
        return false;
    }

    if( !_canBeRestored( content->getType( )))
        return false;

    // Some regular textures were saved as DynamicTexture type before the
    // migration to qml2 rendering
    if( content->getType() == CONTENT_TYPE_DYNAMIC_TEXTURE )
    {
        const QString& uri = content->getURI();
        const auto type = ContentFactory::getContentTypeForFile( uri );
        if( type == CONTENT_TYPE_TEXTURE )
        {
            put_flog( LOG_DEBUG, "Try restoring legacy DynamicTexture as "
                                 "a regular texture: '%s'",
                      content->getURI().toLocal8Bit().constData( ));

            content = ContentFactory::getContent( uri );
            window->setContent( content );
        }
        else
        {
            put_flog( LOG_INFO, "DynamicTexture are no longer supported. Please"
                                "convert the source image to a tiff pyramid: "
                                "'%s'",
                      content->getURI().toLocal8Bit().constData( ));
        }
    }

    // Refresh content information, files can have been modified or removed
    // since the state was saved.
    if( content->readMetadata( ))
    {
        put_flog( LOG_DEBUG, "Restoring content: '%s'",
                  content->getURI().toLocal8Bit().constData( ));
    }
    else
    {
        put_flog( LOG_WARN, "'%s' could not be restored!",
                  content->getURI().toLocal8Bit().constData( ));
        const QSize& size = content->getDimensions();
        window->setContent( ContentFactory::getErrorContent( size ));
    }
    return true;
}

void _validateContents( DisplayGroup& group )
{
    typedef QVector<ContentWindowPtr> Windows;
    Windows windows = Windows::fromStdVector( group.getContentWindows( ));

    QtConcurrent::blockingFilter( windows, _validateContent );

    group.setContentWindows( windows.toStdVector( ));
}

DisplayGroupConstPtr _load( const QString& filename, DisplayGroupConstPtr targetGroup )
{
    State state;
    // For backward compatibility, try to load the file as a legacy xml first
    if( !state.legacyLoadXML( filename ) &&
        !serialization::fromXmlFile( state, filename.toStdString( )))
    {
        return DisplayGroupConstPtr();
    }

    DisplayGroupPtr group = state.getDisplayGroup();
    _validateContents( *group );

    DisplayGroupController controller( *group );
    controller.updateFocusedWindowsCoordinates();

    if( state.getVersion() < FIRST_PIXEL_COORDINATES_FILE_VERSION )
        controller.denormalize( targetGroup->getCoordinates().size( ));
    else if( state.getVersion() == FIRST_PIXEL_COORDINATES_FILE_VERSION )
    {
        // Approximation; only applies to FIRST_PIXEL_COORDINATES_FILE_VERSION
        // which did not serialize the size of the DisplayGroup
        assert( group->getCoordinates().isEmpty( ));
        group->setCoordinates( controller.estimateSurface( ));
    }

    // Reshape the new DisplayGroup only if it doesn't fit (legacy behaviour).
    // If the saved group was smaller, resize it but don't modify its windows.
    if( !targetGroup->getCoordinates().contains( group->getCoordinates( )))
        controller.reshape( targetGroup->getCoordinates().size( ));
    else
    {
        group->setWidth( targetGroup->width( ));
        group->setHeight( targetGroup->height( ));
    }
    group->moveToThread( targetGroup->thread( ));

    return group;
}

QFuture<DisplayGroupConstPtr>
StateSerializationHelper::load( const QString& filename )
{
    put_flog( LOG_INFO, "Restoring session: '%s'",
              filename.toStdString().c_str( ));

    DisplayGroupConstPtr targetGroup = _displayGroup;
    return QtConcurrent::run([targetGroup, filename]()
    {
        return _load( filename, targetGroup );
    });
}

void _generatePreview( const DisplayGroup& group, const QString& filename )
{
    const QSize size = group.size().toSize();
    const ContentWindowPtrs& windows = group.getContentWindows();

    StatePreview filePreview( filename );
    filePreview.generateImage( size, windows );
    filePreview.saveToFile();
}

void _filterContents( DisplayGroup& group )
{
    const auto& windows = group.getContentWindows();

    ContentWindowPtrs filteredWindows;
    filteredWindows.reserve( windows.size( ));

    std::copy_if( windows.begin(), windows.end(),
                  std::back_inserter( filteredWindows ),
                  []( const ContentWindowPtr& window )
    {
        return _canBeRestored( window->getContent()->getType( ));
    });
    group.setContentWindows( filteredWindows );
}

QFuture<bool> StateSerializationHelper::save( QString filename,
                                              const bool generatePreview )
{
    if( !filename.endsWith( SESSION_FILE_EXTENSION ))
    {
        filename.append( SESSION_FILE_EXTENSION );
        put_flog( LOG_VERBOSE, "appended %s filename extension",
                  SESSION_FILE_EXTENSION.toLocal8Bit().constData( ));
    }

    put_flog( LOG_INFO, "Saving session: '%s'",
              filename.toStdString().c_str( ));

    // Important: use xml archive not binary as they use different code paths
    DisplayGroupPtr group = serialization::xmlCopy( _displayGroup );
    return QtConcurrent::run([group, filename, generatePreview]()
    {
        _filterContents( *group );

        // Create preview before session so that thumbnail shows in file browser
        if( generatePreview )
            _generatePreview( *group, filename );

        if( !serialization::toXmlFile( State{ group }, filename.toStdString( )))
            return false;

        return true;
    });
}
