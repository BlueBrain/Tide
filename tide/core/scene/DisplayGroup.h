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

#ifndef DISPLAYGROUP_H
#define DISPLAYGROUP_H

#include "types.h"
#include "Rectangle.h"       // Base class
#include "ContentWindow.h"   // member, needed for serialization
#include "serialization/includes.h"

#include <boost/enable_shared_from_this.hpp>

#include <QUuid>

/**
 * The different versions of the xml serialized display group.
 */
enum DisplayGroupVersion
{
    INVALID_DISPLAYGROUP_VERSION = -1,
    FIRST_DISPLAYGROUP_VERSION = 0
};

/**
 * A collection of ContentWindows.
 *
 * Can be serialized and distributed to the Wall applications.
 */
class DisplayGroup : public Rectangle,
        public boost::enable_shared_from_this<DisplayGroup>
{
    Q_OBJECT
    Q_DISABLE_COPY( DisplayGroup )

    Q_PROPERTY( bool hasFocusedWindows READ hasFocusedWindows
                NOTIFY hasFocusedWindowsChanged )
    Q_PROPERTY( bool hasFullscreenWindows READ hasFullscreenWindows
                NOTIFY hasFullscreenWindowsChanged )
    Q_PROPERTY( ContentWindow* fullscreenWindow READ getFullscreenWindow
                NOTIFY hasFullscreenWindowsChanged )
    Q_PROPERTY( bool hasVisiblePanels READ hasVisiblePanels
                NOTIFY hasVisiblePanelsChanged )

public:
    /** Constructor */
    DisplayGroup( const QSizeF& size );

    /** Destructor */
    virtual ~DisplayGroup();

    /** Add a content window. */
    void addContentWindow( ContentWindowPtr window );

    /** Remove a content window. */
    Q_INVOKABLE void removeContentWindow( ContentWindowPtr window );

    /** Move a content window to the front. */
    void moveToFront( ContentWindowPtr window );

    /**
     * Is the DisplayGroup empty.
     * @return true if the DisplayGroup has no ContentWindow, false otherwise.
     */
    bool isEmpty() const;

    /** Get all windows. */
    const ContentWindowPtrs& getContentWindows() const;

    /** Get a single window by its id. */
    ContentWindowPtr getContentWindow( const QUuid& id ) const;

    /**
     * Replace the content windows.
     * @param windows the list of windows to set.
     */
    void setContentWindows( ContentWindowPtrs windows );

    /** Clear all ContentWindows. */
    void clear();

    /**
     * Get the z index of a window
     * @param window which is part of the DisplayGroup
     * @return the z value of the window, or -1 if it is not part of the
     *         DisplayGroup
     */
    int getZindex( ContentWindowPtr window ) const;

    /** Are there focused windows. */
    bool hasFocusedWindows() const;

    /** Is there a fullscreen window. */
    bool hasFullscreenWindows() const;

    /** Is there any visible panel window(s). */
    bool hasVisiblePanels() const;


    /** Get the fullscreen window (if any). */
    ContentWindow* getFullscreenWindow() const;

    /** Set the fullscreen window. */
    void setFullscreenWindow( ContentWindowPtr window );


    /** Get the set of focused windows. */
    const ContentWindowSet& getFocusedWindows() const;

    /** Add a window to the set of focused windows. */
    void addFocusedWindow( ContentWindowPtr window );

    /** Remove a window from the set of focused windows. */
    void removeFocusedWindow( ContentWindowPtr window );


    /** Get the set of panels. */
    const ContentWindowSet& getPanels() const;

    /**
     * Move this object and its member QObjects to the given QThread.
     *
     * This intentionally shadows the default QObject::moveToThread to include
     * member QObjects which are stored using shared_ptr and thus can't be made
     * direct children of this class.
     * @param thread the target thread.
     */
    void moveToThread( QThread* thread );

    /** @name For session serialization, on Master application only */
    //@{
    /** @return true if window titles were visible when session was saved. */
    bool getShowWindowTitles() const;

    /** Enable/Disable visibility of window titles when saving the session. */
    void setShowWindowTitles( bool set );
    //@}

signals:
    /** Emitted whenever the DisplayGroup is modified */
    void modified( DisplayGroupPtr displayGroup );

    /** Emitted when a content window is added. */
    void contentWindowAdded( ContentWindowPtr contentWindow );

    /** Emitted when a content window is removed. */
    void contentWindowRemoved( ContentWindowPtr contentWindow );

    /** Emitted when a content window is moved to the front. */
    void contentWindowMovedToFront( ContentWindowPtr contentWindow );

    /** @name QProperty notifiers */
    //@{
    /** Notifier for the hasFocusedWindows property. */
    void hasFocusedWindowsChanged();

    /** Notifier for the hasFullscreenWindows property. */
    void hasFullscreenWindowsChanged();

    /** Notifier for the hasVisiblePanels property. */
    void hasVisiblePanelsChanged();
    //@}

private:
    friend class boost::serialization::access;

    /** No-argument constructor required for serialization. */
    DisplayGroup();

    template< class Archive >
    void serialize( Archive & ar, const unsigned int )
    {
        ar & _coordinates;
        ar & _contentWindows;
        ar & _focusedWindows;
        ar & _fullscreenWindow;
        ar & _panels;
    }

    /** Serialize for saving to an xml file */
    template< class Archive >
    void serialize_members_xml( Archive & ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp( "showWindowTitles",
                                             _showWindowTitlesInSavedSession );
        ar & boost::serialization::make_nvp( "contentWindows",
                                             _contentWindows );
        ar & boost::serialization::make_nvp( "coordinates", _coordinates );
    }

    /** Loading from xml. */
    void serialize_for_xml( boost::archive::xml_iarchive& ar,
                            const unsigned int version)
    {
        serialize_members_xml( ar, version );
        for( const auto& window : _contentWindows )
        {
            if( window->isFocused( ))
                _focusedWindows.insert( window );
            if( window->isPanel( ))
                _panels.insert( window );
            // Make sure windows are not in an undefined state
            window->setState( ContentWindow::NONE );
        }
    }

    /** Saving to xml. */
    void serialize_for_xml( boost::archive::xml_oarchive& ar,
                            const unsigned int version )
    {
        serialize_members_xml( ar, version );
    }

    void _sendDisplayGroup();
    void _watchChanges( ContentWindow& window );

    bool _showWindowTitlesInSavedSession = true;
    ContentWindowPtrs _contentWindows;
    ContentWindowSet _focusedWindows;
    ContentWindowPtr _fullscreenWindow;
    ContentWindowSet _panels;

    ContentWindow::WindowMode _fullscreenWindowPrevMode =
            ContentWindow::WindowMode::STANDARD;
    QRectF _fullscreenWindowPrevZoom;
};

BOOST_CLASS_VERSION( DisplayGroup, FIRST_DISPLAYGROUP_VERSION )

DECLARE_SERIALIZE_FOR_XML( DisplayGroup )

#endif
