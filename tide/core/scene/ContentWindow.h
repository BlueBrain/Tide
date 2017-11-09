/*********************************************************************/
/* Copyright (c) 2011-2012, The University of Texas at Austin.       */
/* Copyright (c) 2013-2017, EPFL/Blue Brain Project                  */
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
/*                          Daniel.Nachbaur@epfl.ch                  */
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

#ifndef CONTENTWINDOW_H
#define CONTENTWINDOW_H

#include "types.h"

#include "Content.h"   // member, needed for serialization
#include "Rectangle.h" // base class
#include "serialization/includes.h"

#include <QUuid>

/**
 * A window for displaying Content on the Wall.
 *
 * Can be serialized and distributed to the Wall applications.
 */
class ContentWindow : public Rectangle
{
    Q_OBJECT
    Q_PROPERTY(QUuid id READ getID CONSTANT)
    Q_PROPERTY(bool isPanel READ isPanel CONSTANT)
    Q_PROPERTY(Content* content READ getContentPtr CONSTANT)
    Q_PROPERTY(WindowMode mode READ getMode NOTIFY modeChanged)
    Q_PROPERTY(bool focused READ isFocused NOTIFY modeChanged)
    Q_PROPERTY(QRectF focusedCoordinates READ getFocusedCoordinates NOTIFY
                   focusedCoordinatesChanged)
    Q_PROPERTY(bool fullscreen READ isFullscreen NOTIFY modeChanged)
    Q_PROPERTY(QRectF fullscreenCoordinates READ getFullscreenCoordinates NOTIFY
                   fullscreenCoordinatesChanged)
    Q_PROPERTY(
        WindowState state READ getState WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(ResizeHandle activeHandle READ getActiveHandle WRITE
                   setActiveHandle NOTIFY activeHandleChanged)
    Q_PROPERTY(ResizePolicy resizePolicy READ getResizePolicy WRITE
                   setResizePolicy NOTIFY resizePolicyChanged)
    Q_PROPERTY(
        bool selected READ isSelected WRITE setSelected NOTIFY selectedChanged)

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
    Q_ENUMS(ResizeHandle)

    /** The policy for the active resize operation. */
    enum ResizePolicy
    {
        KEEP_ASPECT_RATIO, // adjust the window aspect ratio to the content's
        ADJUST_CONTENT     // only for compatible contents
    };
    Q_ENUMS(ResizePolicy)

    /** The possible rendering modes of a window. */
    enum WindowMode
    {
        STANDARD,  // standard window mode on the desktop
        FOCUSED,   // window focused for presentation
        FULLSCREEN // fullscreen
    };
    Q_ENUMS(WindowMode)

    /** The possible states of a window. */
    enum WindowState
    {
        NONE,     // not selected, interaction modifies position/size
        MOVING,   // the window is being moved
        RESIZING, // the window is being resized
        HIDDEN    // the window is hidden (invisible, not interacting)
    };
    Q_ENUMS(WindowState)

    /** The different types of windows. */
    enum WindowType
    {
        DEFAULT, // A regular window
        PANEL    // An overlay window without a control bar, cannot be
                 // focused or fullscreen
    };
    Q_ENUMS(WindowType)

    /**
     * Create a new window.
     * @param content The Content to be displayed.
     * @param type The type of the window, which determines its representation
     *        style and behaviour.
     * @note Rank0 only.
     */
    ContentWindow(ContentPtr content, WindowType type = DEFAULT);

    /** Destructor. */
    ~ContentWindow();

    /** @return the unique identifier for this window. */
    const QUuid& getID() const;

    /** @return true if window is a panel. */
    bool isPanel() const;

    /** Get the content from QML. */
    Content* getContentPtr() const;

    /** Get the content. */
    ContentPtr getContent() const;

    /** Set the content, replacing the existing one. @note Rank0 only. */
    void setContent(ContentPtr content);

    /** Set the coordinates in pixel units. */
    void setCoordinates(const QRectF& coordinates);

    /** @return the current active resize handle. */
    ContentWindow::ResizeHandle getActiveHandle() const;

    /** Get the current resize policy. */
    ContentWindow::ResizePolicy getResizePolicy() const;

    /** Get the current state. */
    ContentWindow::WindowMode getMode() const;

    /** Set the current mode. */
    void setMode(const ContentWindow::WindowMode mode);

    /** Is the window focused. */
    bool isFocused() const;

    /** Is the window fullscreen. */
    bool isFullscreen() const;

    /** @return the focused coordinates of this window. */
    const QRectF& getFocusedCoordinates() const;

    /** Set the focused coordinates of this window. */
    void setFocusedCoordinates(const QRectF& coordinates);

    /** @return the fullscreen coordinates of this window. */
    const QRectF& getFullscreenCoordinates() const;

    /** Set the fullscreen coordinates of this window. */
    void setFullscreenCoordinates(const QRectF& coordinates);

    /** @return the actual display coordinates (depending on the mode). */
    const QRectF& getDisplayCoordinates() const;

    /** Set the actual display coordinates (depending on the mode). */
    void setDisplayCoordinates(const QRectF& coordinates);

    /** Get the current state. */
    ContentWindow::WindowState getState() const;

    /** Check if moving. */
    bool isMoving() const;

    /** Check if resizing. */
    bool isResizing() const;

    /** Check if hidden. */
    bool isHidden() const;

    /** @return true if the window is selected. */
    bool isSelected() const;

    /** Select or deselect the window. */
    void setSelected(bool value);

    /** @return the version of the window to apply changes on Wall processes. */
    size_t getVersion() const;

    /** Backup the mode and zoom rectangle (before making fullscreen). */
    void backupModeAndZoom();

    /** Restore the mode and zoom rectangle (after exiting fullscreen). */
    void restoreModeAndZoom();

public slots:
    /** Set the current active resize handle. */
    void setActiveHandle(ContentWindow::ResizeHandle handle);

    /** Set the resize policy. */
    bool setResizePolicy(ContentWindow::ResizePolicy policy);

    /** Set the current state. */
    bool setState(const ContentWindow::WindowState state);

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
    void modeChanged();
    void focusedCoordinatesChanged();
    void fullscreenCoordinatesChanged();
    void stateChanged();
    void selectedChanged();
    void hiddenChanged(bool hidden);
    //@}

private:
    friend class boost::serialization::access;

    /** No-argument constructor required for serialization. */
    ContentWindow();

    /** Serialize for sending to Wall applications. */
    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        // clang-format off
        ar & _coordinates;
        ar & _type;
        ar & _uuid;
        ar & _content;
        ar & _activeHandle;
        ar & _resizePolicy;
        ar & _mode;
        ar & _focusedCoordinates;
        ar & _fullscreenCoordinates;
        ar & _windowState;
        ar & _selected;
        ar & _version;
        // clang-format on
    }

    /** Serialize members to and from xml. */
    template <class Archive>
    void serialize_members_xml(Archive& ar, const unsigned int version)
    {
        // clang-format off
        ar & boost::serialization::make_nvp("content", _content);
        if (version < 1)
        {
            int contentWidth = 0, contentHeight = 0;
            ar & boost::serialization::make_nvp("contentWidth", contentWidth);
            ar & boost::serialization::make_nvp("contentHeight", contentHeight);
        }
        ar & boost::serialization::make_nvp("coordinates", _coordinates);
        if (version < 3)
        {
            QRectF backup;
            ar & boost::serialization::make_nvp("coordinatesBackup", backup);
        }
        QPointF zoomCenter = _content->getZoomRect().center();
        qreal zoom = 1.0 / _content->getZoomRect().width();
        ar & boost::serialization::make_nvp("centerX", zoomCenter.rx());
        ar & boost::serialization::make_nvp("centerY", zoomCenter.ry());
        ar & boost::serialization::make_nvp("zoom", zoom);
        if (version >= 3)
        {
            bool focused = isFocused();
            ar & boost::serialization::make_nvp("focused", focused);
            if (focused)
                setMode(WindowMode::FOCUSED);
        }
        QRectF zoomRect;
        zoomRect.setSize(QSizeF(1.0 / zoom, 1.0 / zoom));
        zoomRect.moveCenter(zoomCenter);
        _content->setZoomRect(zoomRect);
        if (version < 1)
        {
            int controlState = 0;
            ar & boost::serialization::make_nvp("controlState", controlState);
        }
        ar & boost::serialization::make_nvp("windowState", _windowState);
        // clang-format on
    }

    /** Loading from xml. */
    void serialize_for_xml(boost::archive::xml_iarchive& ar,
                           const unsigned int version)
    {
        serialize_members_xml(ar, version);
        _init();
    }

    /** Saving to xml. */
    void serialize_for_xml(boost::archive::xml_oarchive& ar,
                           const unsigned int version)
    {
        serialize_members_xml(ar, version);
    }

    void _init();

    QUuid _uuid;
    WindowType _type;
    ContentPtr _content;
    ContentWindow::ResizeHandle _activeHandle;
    ContentWindow::ResizePolicy _resizePolicy;
    ContentWindow::WindowMode _mode;
    QRectF _focusedCoordinates;
    QRectF _fullscreenCoordinates;
    ContentWindow::WindowState _windowState;
    bool _selected;
    size_t _version;

    WindowMode _backupMode = WindowMode::STANDARD;
    QRectF _backupZoom;
};

BOOST_CLASS_VERSION(ContentWindow, 3)

DECLARE_SERIALIZE_FOR_XML(ContentWindow)

#endif
