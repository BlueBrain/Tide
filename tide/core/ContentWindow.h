/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
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

#ifndef CONTENT_WINDOW_H
#define CONTENT_WINDOW_H

#include "types.h"

#include "Coordinates.h"
#include "Content.h" // needed for serialization
#include "ContentWindowController.h" // needed for serialization
#include "serializationHelpers.h"

#include <QObject>
#include <QUuid>
#include <QRectF>

#ifndef Q_MOC_RUN
// https://bugreports.qt.nokia.com/browse/QTBUG-22829: When Qt moc runs on CGAL
// files, do not process <boost/type_traits/has_operator.hpp>
#  include <boost/serialization/shared_ptr.hpp>
#  include <boost/serialization/scoped_ptr.hpp>
#endif

/**
 * A window for displaying Content on the Wall.
 *
 * Can be serialized and distributed to the Wall applications.
 */
class ContentWindow : public Coordinates
{
    Q_OBJECT
    Q_PROPERTY( QUuid id READ getID )
    Q_PROPERTY( bool isPanel READ isPanel CONSTANT )
    Q_PROPERTY( Content* content READ getContentPtr CONSTANT )
    Q_PROPERTY( WindowState state READ getState WRITE setState
                NOTIFY stateChanged )
    Q_PROPERTY( ResizeHandle activeHandle READ getActiveHandle
                WRITE setActiveHandle NOTIFY activeHandleChanged )
    Q_PROPERTY( ResizePolicy resizePolicy READ getResizePolicy
                WRITE setResizePolicy NOTIFY resizePolicyChanged )
    Q_PROPERTY( bool focused READ isFocused WRITE setFocused
                NOTIFY focusedChanged )
    Q_PROPERTY( QString label READ getLabel NOTIFY labelChanged )
    Q_PROPERTY( bool controlsVisible READ getControlsVisible
                WRITE setControlsVisible NOTIFY controlsVisibleChanged )
    Q_PROPERTY( ContentInteractionDelegate* delegate READ getInteractionDelegate
                CONSTANT )
    Q_PROPERTY( ContentWindowController* controller READ getController
                CONSTANT )
    Q_PROPERTY( QRectF focusedCoordinates READ getFocusedCoordinates
                WRITE setFocusedCoordinates NOTIFY focusedCoordinatesChanged )

public:
    /** The current active resize handle. */
    enum ResizeHandle
    {
        TOP_LEFT,
        TOP,
        TOP_RIGHT,
        RIGHT,
        BOTTOM_RIGHT,
        BOTTOM,
        BOTTOM_LEFT,
        LEFT,
        NOHANDLE
    };
    Q_ENUMS( ResizeHandle )

    /** The policy for the active resize operation. */
    enum ResizePolicy
    {
        KEEP_ASPECT_RATIO,   // adjust the window aspect ratio to the content's
        ADJUST_CONTENT       // only for compatible contents
    };
    Q_ENUMS( ResizePolicy )

    /** The possible states of a window. */
    enum WindowState
    {
        NONE,       // not selected, interaction modifies position/size
        MOVING,     // the window is being moved
        RESIZING,   // the window is being resized
        HIDDEN      // the window is hidden (invisible, not interacting)
    };
    Q_ENUMS( WindowState )

    /** The different types of windows. */
    enum WindowType
    {
        DEFAULT,    // A regular window
        PANEL       // A panel window - always interative, cannot be focused
    };
    Q_ENUMS( WindowType )

    /**
     * Create a new window.
     * @param content The Content to be displayed.
     * @param type The type of the window, which determines its representation
     *        style and behaviour.
     * @note Rank0 only.
     */
    ContentWindow( ContentPtr content, WindowType type = DEFAULT );

    /** Destructor. */
    ~ContentWindow();

    /** @return the unique identifier for this window. */
    const QUuid& getID() const;

    /** Is the window a panel */
    bool isPanel() const;

    /** Get the content from QML. */
    Content* getContentPtr() const;

    /** Get the content. */
    ContentPtr getContent() const;

    /** Set the content, replacing the existing one. @note Rank0 only. */
    void setContent( ContentPtr content );


    /** @return the controller for this window, or 0 if the window has not been
     *          added to a DisplayGroup.
     */
    ContentWindowController* getController();

    /** @return the controller for this window, or 0 if the window has not been
     *          added to a DisplayGroup.
     */
    const ContentWindowController* getController() const;

    /** Assign a controller. */
    void setController( ContentWindowControllerPtr controller );


    /** Set the coordinates in pixel units. */
    void setCoordinates( const QRectF& coordinates );

    /** @return the current active resize handle. */
    ContentWindow::ResizeHandle getActiveHandle() const;

    /** Get the current resize policy. */
    ContentWindow::ResizePolicy getResizePolicy() const;

    /** Get the current state. */
    ContentWindow::WindowState getState() const;

    /** Is the window focused. */
    bool isFocused() const;

    /** Set the window in focused mode. */
    void setFocused( bool value );

    /** @return the focused coordinates of this window. */
    const QRectF& getFocusedCoordinates() const;

    /** Set the focused coordinates of this window. */
    void setFocusedCoordinates( const QRectF& coordinates );


    /** @return the actual coordinates of this window (normal or focused). */
    const QRectF& getDisplayCoordinates() const;


    /** Set the current state. */
    bool setState( const ContentWindow::WindowState state );

    /** Check if moving. */
    bool isMoving() const;

    /** Check if resizing. */
    bool isResizing() const;

    /** Check if hidden. */
    bool isHidden() const;


    /**
     * Get the interaction delegate.
     * @note Rank0 only.
     */
    ContentInteractionDelegate* getInteractionDelegate();

    /** Get the label for the window */
    QString getLabel() const;

    /** Get the visibility of the window control buttons. */
    bool getControlsVisible() const;

    /** Set the visibility of the window control buttons. */
    void setControlsVisible( bool value );

public slots:
    /** Set the current active resize handle. */
    void setActiveHandle( ContentWindow::ResizeHandle handle );

    /** Set the resize policy. */
    bool setResizePolicy( ContentWindow::ResizePolicy policy );

signals:
    /** Emitted when the Content signals that it has been modified. */
    void contentModified();

    /**
     * Emitted whenever this object is modified.
     * Used by DisplayGroup on Rank0 to distibute changes to the other ranks.
     */
    void modified();

    /** Emitted when setCoordinates has been called. */
    void coordinatesChanged();

    /** @name QProperty notifiers */
    //@{
    void activeHandleChanged();
    void resizePolicyChanged();
    void focusedChanged();
    void focusedCoordinatesChanged();
    void stateChanged();
    void labelChanged();
    void controlsVisibleChanged();
    //@}

private:
    friend class boost::serialization::access;

    /** No-argument constructor required for serialization. */
    ContentWindow();

    /** Serialize for sending to Wall applications. */
    template< class Archive >
    void serialize( Archive & ar, const unsigned int )
    {
        ar & _coordinates;
        ar & _type;
        ar & _uuid;
        ar & _content;
        ar & _controller;
        ar & _activeHandle;
        ar & _resizePolicy;
        ar & _focused;
        ar & _focusedCoordinates;
        ar & _windowState;
        ar & _controlsVisible;
    }

    /** Serialize members to and from xml. */
    template< class Archive >
    void serialize_members_xml( Archive & ar, const unsigned int version )
    {
        ar & boost::serialization::make_nvp( "content", _content );
        if( version < 1 )
        {
            int contentWidth = 0, contentHeight = 0;
            ar & boost::serialization::make_nvp( "contentWidth", contentWidth );
            ar & boost::serialization::make_nvp( "contentHeight", contentHeight );
        }
        ar & boost::serialization::make_nvp( "coordinates", _coordinates );
        if( version < 3 )
        {
            QRectF backup;
            ar & boost::serialization::make_nvp( "coordinatesBackup", backup );
        }
        QPointF zoomCenter = _content->getZoomRect().center();
        qreal zoom = 1.0 / _content->getZoomRect().width();
        ar & boost::serialization::make_nvp( "centerX", zoomCenter.rx( ));
        ar & boost::serialization::make_nvp( "centerY", zoomCenter.ry( ));
        ar & boost::serialization::make_nvp( "zoom", zoom );
        if( version >= 3 )
            ar & boost::serialization::make_nvp( "focused", _focused );
        QRectF zoomRect;
        zoomRect.setSize( QSizeF( 1.0/zoom, 1.0/zoom ));
        zoomRect.moveCenter( zoomCenter );
        _content->setZoomRect( zoomRect );
        if( version < 1 )
        {
            int controlState = 0;
            ar & boost::serialization::make_nvp( "controlState", controlState );
        }
        ar & boost::serialization::make_nvp( "windowState", _windowState );
    }

    /** Loading from xml. */
    void serialize_for_xml( boost::archive::xml_iarchive& ar,
                            const unsigned int version )
    {
        serialize_members_xml( ar, version );
        _init();
    }

    /** Saving to xml. */
    void serialize_for_xml( boost::archive::xml_oarchive& ar,
                            const unsigned int version )
    {
        serialize_members_xml( ar, version );
    }

    void _init();
    void _createInteractionDelegate();

    QUuid _uuid;
    WindowType _type;
    ContentPtr _content;
    ContentWindowController* _controller; // child QObject, don't delete
    ContentWindow::ResizeHandle _activeHandle;
    ContentWindow::ResizePolicy _resizePolicy;
    bool _focused;
    QRectF _focusedCoordinates;
    ContentWindow::WindowState _windowState;
    bool _controlsVisible;

    boost::scoped_ptr<ContentInteractionDelegate> _interactionDelegate;
};

BOOST_CLASS_VERSION( ContentWindow, 3 )

DECLARE_SERIALIZE_FOR_XML( ContentWindow )

#endif
