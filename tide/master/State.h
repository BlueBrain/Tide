/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
/*                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>     */
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
#ifndef STATE_H
#define STATE_H

#include "types.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include "ContentWindow.h"
#include "DisplayGroup.h"

class QString;
class QXmlQuery;

/**
 * The different versions of the xml State files.
 */
enum StateVersion
{
    INVALID_FILE_VERSION = -1,
    FIRST_BOOST_FILE_VERSION = 0,
    LEGACY_FILE_VERSION = 1,
    FIRST_PIXEL_COORDINATES_FILE_VERSION = 2,
    WINDOW_TITLES_VERSION = 3,
    FOCUS_MODE_VERSION = 4
};

/**
 * A state is the collection of opened contents which can be saved and
 * restored using this class. It will save positions and dimensions of each
 * content and also content-specific information which is required to restore
 * a previous state saved by the user.
 */
class State
{
public:
    /** Default constructor */
    State();
    /**
     * Constructor
     * @param displayGroup the DisplayGroup to serialize
     */
    State( DisplayGroupPtr displayGroup );

    /**
     * Load content windows stored in the given XML file.
     * @return success if the legacy state file could be loaded
     */
    bool legacyLoadXML( const QString& filename );

    /** Get the version that was last used for loading or saving. */
    StateVersion getVersion() const;

    /** Get the DisplayGroup. */
    DisplayGroupPtr getDisplayGroup();

private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize( Archive & ar, const unsigned int version )
    {
        _version = static_cast< StateVersion >( version );

        if( version < FIRST_PIXEL_COORDINATES_FILE_VERSION )
            _displayGroup->setCoordinates( UNIT_RECTF );

        if( version < WINDOW_TITLES_VERSION )
        {
            ContentWindowPtrs contentWindows;
            ar & boost::serialization::make_nvp( "contentWindows",
                                                 contentWindows );
            _displayGroup->setContentWindows( contentWindows );
            _displayGroup->setShowWindowTitles( false );
        }
        else
        {
            ar & boost::serialization::make_nvp( "displayGroup",
                                                 _displayGroup );
        }
    }

    /** Legacy methods. @deprecated */
    bool _checkVersion( QXmlQuery& query ) const;
    ContentPtr _loadContent( QXmlQuery& query, const int index ) const;
    ContentWindowPtr _restoreContent( QXmlQuery& query, ContentPtr content,
                                      const int index ) const;

    DisplayGroupPtr _displayGroup;
    StateVersion _version;
};

BOOST_CLASS_VERSION( State, FOCUS_MODE_VERSION )

#endif
