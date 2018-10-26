/*********************************************************************/
/* Copyright (c) 2011-2012, The University of Texas at Austin.       */
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
/*                          Raphael.Dumusc@epfl.ch                   */
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

#ifndef DISPLAYGROUP_H
#define DISPLAYGROUP_H

#include "Rectangle.h" // Base class
#include "Window.h"    // member, needed for serialization
#include "serialization/includes.h"
#include "types.h"

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
 * A collection of Windows.
 *
 * Can be serialized and distributed to the Wall applications.
 */
class DisplayGroup : public Rectangle,
                     public std::enable_shared_from_this<DisplayGroup>
{
    Q_OBJECT
    Q_DISABLE_COPY(DisplayGroup)

    Q_PROPERTY(bool empty READ isEmpty NOTIFY isEmptyChanged)
    Q_PROPERTY(bool hasFocusedWindows READ hasFocusedWindows NOTIFY
                   hasFocusedWindowsChanged)
    Q_PROPERTY(bool hasFullscreenWindows READ hasFullscreenWindows NOTIFY
                   hasFullscreenWindowsChanged)
    Q_PROPERTY(Window* fullscreenWindow READ getFullscreenWindow NOTIFY
                   hasFullscreenWindowsChanged)
    Q_PROPERTY(bool hasVisiblePanels READ hasVisiblePanels NOTIFY
                   hasVisiblePanelsChanged)
    Q_PROPERTY(QStringList selectedUris READ getSelectedUris NOTIFY
                   selectedUrisChanged)

public:
    static DisplayGroupPtr create(const QSizeF& size);

    /** Destructor */
    virtual ~DisplayGroup();

    /** Add a window. */
    void add(WindowPtr window);

    /** Remove a window. */
    void remove(WindowPtr window);

    /** Move a window to the front. */
    void moveToFront(WindowPtr window);

    /** @return true if the DisplayGroup has no windows, false otherwise. */
    bool isEmpty() const;

    /** Get all windows. */
    const WindowPtrs& getWindows() const;

    /** Get a single window by its id. */
    WindowPtr getWindow(const QUuid& id) const;

    /** Find a single window based on its filename. */
    WindowPtr findWindow(const QString& filename) const;

    /**
     * Replace the windows by a new list.
     * @param windows the list of windows to set.
     */
    void replaceWindows(WindowPtrs windows);

    /** Remove all windows. */
    void clear();

    /**
     * Get the z index of a window.
     * @param id of a window which is part of the DisplayGroup.
     * @return the z index of the window, or -1 if it is not part of the
     *         DisplayGroup.
     */
    int getZindex(const QUuid& id) const;

    /** Are there focused windows. */
    bool hasFocusedWindows() const;

    /** Is there a fullscreen window. */
    bool hasFullscreenWindows() const;

    /** Is there any visible panel window(s). */
    bool hasVisiblePanels() const;

    /** Get the fullscreen window (if any). */
    Window* getFullscreenWindow() const;

    /** Set the fullscreen window. */
    void setFullscreenWindow(WindowPtr window);

    /** Get the set of focused windows. */
    const WindowSet& getFocusedWindows() const;

    /** Add a window to the set of focused windows. */
    void addFocusedWindow(WindowPtr window);

    /** Remove a window from the set of focused windows. */
    void removeFocusedWindow(WindowPtr window);

    /** Get the set of panels. */
    const WindowSet& getPanels() const;

    /** Get the set of selected windows. */
    WindowSet getSelectedWindows() const;

    /** Get the set of windows that can be focused (all excluding panels). */
    WindowSet getFocusableWindows() const;

    /** @return the URIs of the selected windows. */
    QStringList getSelectedUris() const;

    /**
     * Move this object and its member QObjects to the given QThread.
     *
     * This intentionally shadows the default QObject::moveToThread to include
     * member QObjects which are stored using shared_ptr and thus can't be made
     * direct children of this class.
     * @param thread the target thread.
     */
    void moveToThread(QThread* thread);

signals:
    /** Emitted whenever the DisplayGroup is modified */
    void modified(DisplayGroupPtr displayGroup);

    /** Emitted when a content window is added. */
    void windowAdded(WindowPtr window);

    /** Emitted when a content window is removed. */
    void windowRemoved(WindowPtr window);

    /** Emitted when a content window is moved to the front. */
    void windowMovedToFront(WindowPtr window);

    /** Emitted when the DisplayGroup is cleared. */
    void cleared(uint windowCount);

    /** @name QProperty notifiers */
    //@{
    void isEmptyChanged();
    void hasFocusedWindowsChanged();
    void hasFullscreenWindowsChanged();
    void hasVisiblePanelsChanged();
    void selectedUrisChanged();
    //@}

private:
    friend class boost::serialization::access;

    /** No-argument constructor required for serialization. */
    DisplayGroup() = default;

    DisplayGroup(const QSizeF& size);

    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        // clang-format off
        ar & _coordinates;
        ar & _windows;
        ar & _focusedWindows;
        ar & _fullscreenWindow;
        ar & _panels;
        // clang-format on
    }

    /** Serialize for saving to an xml file */
    template <class Archive>
    void serialize_members_xml(Archive& ar, const unsigned int version)
    {
        // clang-format off
        if (version == FIRST_DISPLAYGROUP_VERSION)
        {
            // legacy field, ignored but not removed yet to maintain full
            // compatibility of sessions with previous Tide versions < 1.3.
            bool _titles = true;
            ar & boost::serialization::make_nvp("showWindowTitles", _titles);
        }
        ar & boost::serialization::make_nvp("contentWindows", _windows);
        ar & boost::serialization::make_nvp("coordinates", _coordinates);
        // clang-format on
    }

    /** Loading from xml. */
    void serialize_for_xml(boost::archive::xml_iarchive& ar,
                           const unsigned int version)
    {
        serialize_members_xml(ar, version);
        for (const auto& window : _windows)
        {
            if (window->isFocused())
                _focusedWindows.insert(window);
            if (window->isPanel())
                _panels.insert(window);
            // Make sure windows are not in an undefined state
            window->setState(Window::NONE);
        }
    }

    /** Saving to xml. */
    void serialize_for_xml(boost::archive::xml_oarchive& ar,
                           const unsigned int version)
    {
        serialize_members_xml(ar, version);
    }

    void _sendDisplayGroup();
    void _watchChanges(Window& window);

    WindowPtrs _windows;
    WindowSet _focusedWindows;
    WindowPtr _fullscreenWindow;
    WindowSet _panels;
};

BOOST_CLASS_VERSION(DisplayGroup, FIRST_DISPLAYGROUP_VERSION)

DECLARE_SERIALIZE_FOR_XML(DisplayGroup)

#endif
