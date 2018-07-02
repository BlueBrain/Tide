/*********************************************************************/
/* Copyright (c) 2013-2018, EPFL/Blue Brain Project                  */
/*                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>*/
/*                          Raphael Dumusc <raphael.dumusc@epfl.ch>  */
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

#include "control/MultiChannelWindowController.h"
#include "scene/ContentType.h"
#include "types.h"

#include <deflect/SizeHints.h>

#include <QObject>
#include <QPointF>
#include <QSize>
#include <QUuid>
#include <map>

/**
 * Handles window creation, association and updates for pixel streamers, both
 * local and external. The association is one streamer to one window.
 */
class PixelStreamWindowManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PixelStreamWindowManager)

public:
    /**
     * Create a window manager that handles windows for streamers.
     *
     * @param scene where the windows of streamers will be added and removed.
     */
    PixelStreamWindowManager(Scene& scene);

    /**
     * @param uri the URI of streamer
     * @return the associated windows of the given streamer. Can be empty.
     * @note this function is public only for the purpose of unit testing.
     * @internal
     */
    WindowPtrs getWindows(const QString& uri) const;

    /**
     * Hide the associated content window of the stream.
     *
     * @param uri the URI of the streamer
     */
    void hideWindows(const QString& uri);

    /**
     * Show the associated content windows of the stream.
     *
     * @param uri the URI of the streamer
     */
    void showWindows(const QString& uri);

    /**
     * Open a window for a new PixelStream.
     *
     * @param surfaceIndex the surface on which to open the window.
     * @param uri the URI of the streamer
     * @param size the desired size of the window in pixels.
     * @param pos the desired position for the center of the window in pixels.
     *        If pos.isNull(), the window is centered on the DisplayGroup.
     * @param stream the type of stream for the window.
     * @throw std::logic_error is size is null and stream is not EXTERNAL.
     */
    void openWindow(uint surfaceIndex, const QString& uri, const QSize& size,
                    const QPointF& pos = QPointF(),
                    StreamType stream = StreamType::EXTERNAL);

    /** Check if new windows open in focus mode. */
    bool getAutoFocusNewWindows() const;

    /**
     * Handle the begining of a stream, opening a window if needed.
     *
     * @param uri the URI of the streamer
     */
    void handleStreamStart(QString uri);

    /**
     * Handle the end of a stream, closing its window.
     *
     * @param uri the URI of the streamer
     */
    void handleStreamEnd(QString uri);

    /**
     * Is called when the streamer wants to enable event handling. This will
     * register the given EventReceiver in the content window.
     *
     * @param uri the URI of the streamer
     * @param exclusive true if only one source of the streamer should
     *        send/handle events
     * @param receiver the event receiver instance
     * @param success promise that will receive the success of the operation
     */
    void registerEventReceiver(QString uri, bool exclusive,
                               deflect::server::EventReceiver* receiver,
                               deflect::server::BoolPromisePtr success);

    /**
     * Update the dimension of the content according to the stream's dimension
     * @param frame the new stream frame to check its dimension
     */
    void updateStreamWindows(deflect::server::FramePtr frame);

    /**
     * Update the size hints for the content, sent by the streamer.
     *
     * @param uri the URI of the streamer
     * @param hints the new size hints to use for the content
     */
    void updateSizeHints(QString uri, deflect::SizeHints hints);

    /**
     * Send data to window.
     *
     * @param uri the URI of the streamer
     * @param data the data to send
     */
    void sendDataToWindow(QString uri, QByteArray data);

    /** Open new windows in focus mode. */
    void setAutoFocusNewWindows(bool set);

signals:
    /**
     * Emitted when the associated content window of the EXTERNAL streamer is
     * created. The stream window is hidden and the user has to call
     * showWindow() or handleStreamEnd().
     *
     * @param uri the URI of the streamer
     */
    void externalStreamOpening(QString uri);

    /**
     * Emitted when the associated content window of the streamer is closed.
     *
     * @param uri the URI of the streamer
     */
    void streamWindowClosed(QString uri);

    /**
     * Emitted when handleStreamStart is called for a stream which already has a
     * window, such as the Launcher or a Webbrowser.
     *
     * For external streamers, the requestFrame signal comes from the Wall
     * processes when the window has been opened. For local streamers, however,
     * the window is opened before the deflect::Stream is started so the
     * deflect::server::FrameDispatcher discards it and no frames would be
     * displayed otherwise.
     *
     * @param uri the URI of the streamer
     */
    void requestFirstFrame(QString uri);

private:
    Scene& _scene;
    MultiChannelWindowController _windowController{_scene};

    struct WindowInfo
    {
        QUuid uuid;
        uint channel = 0;
    };
    using SurfaceToWindowMap = std::map<uint, WindowInfo>;

    struct StreamInfo
    {
        SurfaceToWindowMap windows;
    };
    std::map<QString, StreamInfo> _streams;
    bool _autoFocusNewWindows = true;

    void _monitor(const DisplayGroup& group, uint surfaceIndex);
    void _show(Window& window);
    bool _isWindowOpen(const QString& uri, uint surfaceIndex) const;
    bool _isStreamVisible(const QString& uri) const;
    bool _isValid(const uint surfaceIndex) const;
    void _focus(const Window& window);
    void _onWindowAdded(WindowPtr window, uint surfaceIndex);
    void _onWindowRemoved(WindowPtr window, uint surfaceIndex);
    void _closeWindowsWithoutAChannel(const QString& uri,
                                      const std::set<uint8_t>& channels);
    void _updateWindowSize(Window& window, DisplayGroup& group,
                           const QSize& size);
    void _resizeInPlace(Window& window, const DisplayGroup& group,
                        const QSize& size);
};

#endif
