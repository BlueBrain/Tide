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

#include "ContentFactory.h"
#include "DisplayGroup.h"
#include "DisplayGroupController.h"
#include "State.h"
#include "StatePreview.h"

#include "log.h"

#include <fstream>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_archive_exception.hpp>

StateSerializationHelper::StateSerializationHelper( DisplayGroupPtr displayGroup )
    : _displayGroup( displayGroup )
{
}

bool StateSerializationHelper::save( const QString& filename,
                                     const bool generatePreview )
{
    put_flog( LOG_INFO, "Saving session: '%s'",
              filename.toStdString().c_str( ));

    ContentWindowPtrs contentWindows = _displayGroup->getContentWindows();

    if( generatePreview )
    {
        const QSizeF& size = _displayGroup->getCoordinates().size();
        const QSize dimensions( (int)size.width(), (int)size.height( ));
        StatePreview filePreview( filename );
        filePreview.generateImage( dimensions, contentWindows );
        filePreview.saveToFile();
    }

    // serialize state
    std::ofstream ofs( filename.toStdString( ));
    if ( !ofs.good( ))
        return false;

    // brace this so destructor is called on archive before we use the stream
    {
        State state( _displayGroup );
        boost::archive::xml_oarchive oa( ofs );
        oa << BOOST_SERIALIZATION_NVP( state );
    }
    ofs.close();

    return true;
}

bool _loadBoostXml( State& state, const QString& filename )
{
    // De-serialize state file
    std::ifstream ifs( filename.toStdString( ));
    if ( !ifs.good( ))
        return false;
    try
    {
        boost::archive::xml_iarchive ia( ifs );
        ia >> BOOST_SERIALIZATION_NVP( state );
    }
    catch( const boost::archive::archive_exception& e )
    {
        put_flog( LOG_ERROR, "Could not restore session: '%s': %s",
                  filename.toStdString().c_str(), e.what( ));
        return false;
    }
    catch( const std::exception& e )
    {
        put_flog( LOG_ERROR, "Could not restore state file '%s'',"
                             "wrong file format: %s",
                  filename.toStdString().c_str(), e.what( ));
        return false;
    }

    ifs.close();
    return true;
}

bool StateSerializationHelper::load( const QString& filename )
{
    put_flog( LOG_INFO, "Restoring session: '%s'",
              filename.toStdString().c_str( ));

    State state;
    // For backward compatibility, try to load the file as a legacy xml first
    if( !state.legacyLoadXML( filename ) && !_loadBoostXml( state, filename ))
        return false;

    DisplayGroupPtr group = state.getDisplayGroup();
    _validateContents( *group );

    DisplayGroupController controller( *group );

    if( state.getVersion() < FIRST_PIXEL_COORDINATES_FILE_VERSION )
        controller.denormalize( _displayGroup->getCoordinates().size( ));
    else if( state.getVersion() == FIRST_PIXEL_COORDINATES_FILE_VERSION )
    {
        // Approximation; only applies to FIRST_PIXEL_COORDINATES_FILE_VERSION
        // which did not serialize the size of the DisplayGroup
        assert( group->getCoordinates().isEmpty( ));
        group->setCoordinates( controller.estimateSurface( ));
    }

    // Scale down the new DisplayGroup if it doesn't fit
    if( !_displayGroup->getCoordinates().contains( group->getCoordinates( )))
        controller.adjust( _displayGroup->getCoordinates().size( ));

    _displayGroup->setShowWindowTitles( group->getShowWindowTitles( ));
    _displayGroup->setContentWindows( group->getContentWindows( ));
    return true;
}

void StateSerializationHelper::_validateContents( DisplayGroup& group ) const
{
    const ContentWindowPtrs& windows = group.getContentWindows();

    ContentWindowPtrs validContentWindows;
    validContentWindows.reserve( windows.size( ));

    for( ContentWindowPtr contentWindow : windows )
    {
        ContentPtr content = contentWindow->getContent();
        if( !content )
        {
            put_flog( LOG_WARN, "Window '%s' does not have a Content!",
                contentWindow->getID().toString().toLocal8Bit().constData( ));
            continue;
        }
        // PixelStreams are not supported yet, don't restore them.
        // This feature will be implemented in DISCL-6
        if( content->getType() == CONTENT_TYPE_PIXEL_STREAM )
            continue;

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
                contentWindow->setContent( content );
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
            contentWindow->setContent( ContentFactory::getErrorContent( size ));
        }
        validContentWindows.push_back( contentWindow );
    }

    group.setContentWindows( validContentWindows );
}
