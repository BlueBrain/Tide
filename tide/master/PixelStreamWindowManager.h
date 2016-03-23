/*********************************************************************/
/* Copyright (c) 2014-2015, EPFL/Blue Brain Project                  */
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

#ifndef PIXEL_STREAM_WINDOW_MANAGER_H
#define PIXEL_STREAM_WINDOW_MANAGER_H

#include <QObject>
#include <QPointF>
#include <QSize>
#include <QUuid>
#include <map>

#include "types.h"
#include <deflect/SizeHints.h>

/**
 * Handles window creation, association and updates for pixel streamers, both
 * local and external. The association is one streamer to one window.
 */
class PixelStreamWindowManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Create a window manager that handles windows for streamers.
     *
     * @param displayGroup the content windows of streamers will be added
     *                     to and removed from this DisplayGroup.
     */
    PixelStreamWindowManager( DisplayGroup& displayGroup );

    /**
     * @param uri the URI of streamer
     * @return the associated window of the given streamer. Can be NULL.
     * @todo This should not be public! See DISCL-230
     */
    ContentWindowPtr getContentWindow( const QString& uri ) const;

    /**
     * Hide the associated content window of the stream.
     *
     * @param uri the URI of the streamer
     */
    void hideWindow( const QString& uri );

    /**
     * Show the associated content window of the stream.
     *
     * @param uri the URI of the streamer
     */
    void showWindow( const QString& uri );

    /**
     * Open a window for a new PixelStream.
     *
     * @param uri the URI of the streamer
     * @param pos the desired position for the center of the window in pixels.
     *        If pos.isNull(), the window is centered on the DisplayGroup.
     * @param size the desired size of the window in pixels.
     */
    void openWindow( const QString& uri, const QPointF& pos, const QSize& size);

    /** Check if new windows open in focus mode. */
    bool getAutoFocusNewWindows() const;

public slots:
    /**
     * Open a window for a new external PixelStream.
     *
     * @param uri the URI of the streamer
     */
    void openPixelStreamWindow( QString uri );

    /**
     * Close the window of a PixelStream.
     *
     * @param uri the URI of the streamer
     */
    void closePixelStreamWindow( QString uri );

    /**
     * Is called when the streamer wants to enable event handling. This will
     * register the given EventReceiver in the content window.
     *
     * @param uri the URI of the streamer
     * @param exclusive true if only one source of the streamer should
     *        send/handle events
     * @param receiver the event receiver instance
     */
    void registerEventReceiver( QString uri, bool exclusive,
                                deflect::EventReceiver* receiver );

    /**
     * Update the dimension of the content according to the stream's dimension
     * @param frame the new stream frame to check its dimension
     */
    void updateStreamDimensions( deflect::FramePtr frame );

    /**
     * Update the size hints for the content, sent by the streamer.
     *
     * @param uri the URI of the streamer
     * @param hints the new size hints to use for the content
     */
    void updateSizeHints( QString uri, deflect::SizeHints hints );

    /** Open new windows in focus mode. */
    void setAutoFocusNewWindows( bool set );

signals:
    /**
     * Is emitted when the associated content window of the streamer is closed.
     *
     * @param uri the URI of the streamer
     */
    void pixelStreamWindowClosed( QString uri );

    /**
     * Is emitted after registerEventReceiver() was executed.
     *
     * @param uri the URI of the streamer
     * @param success true if event registration was successful, false otherwise
     */
    void eventRegistrationReply( QString uri, bool success );

private slots:
    /**
     * This will close the streamer that is associated with the given window.
     *
     * @param window the content window that might be associated to a streamer
     */
    void onContentWindowRemoved( ContentWindowPtr window );

private:
    Q_DISABLE_COPY( PixelStreamWindowManager )

    DisplayGroup& _displayGroup;

    typedef std::map< QString, QUuid > ContentWindowMap;
    ContentWindowMap _streamerWindows;

    bool _autoFocusNewWindows;

    bool _isPanel( const QString& uri ) const;
};

#endif
