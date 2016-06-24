/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
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

#ifndef WEBKITPIXELSTREAMER_H
#define WEBKITPIXELSTREAMER_H

#include "PixelStreamer.h"

#include <QString>
#include <QImage>
#include <QTimer>
#include <QWebView>
#include <QMutex>

#include <boost/smart_ptr/scoped_ptr.hpp>

class QRect;
class QWebHitTestResult;
class QWebElement;

class WebkitAuthenticationHelper;
class WebkitHtmlSelectReplacer;

/**
 * Stream webpages with user interaction support.
 */
class WebkitPixelStreamer : public PixelStreamer
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param webpageSize The desired size of the webpage viewport. The actual stream
     *        dimensions will be: size * default zoom factor (2x).
     * @param url The webpage to load.
     */
    WebkitPixelStreamer(const QSize& webpageSize, const QString& url);

    /** Destructor. */
    ~WebkitPixelStreamer();

    /** Get the size of the webpage images. */
    QSize size() const override;

    /**
     * Open a webpage.
     *
     * @param url The address of the webpage to load.
     */
    void setUrl(const QString& url);

    /** Get the QWebView used internally by the streamer. */
    const QWebView* getView() const;

public slots:
    /** Process an Event. */
    void processEvent(deflect::Event event) override;

private slots:
    void _update();

private:
    QWebView _webView;
    boost::scoped_ptr<WebkitAuthenticationHelper> _authenticationHelper;
    boost::scoped_ptr<WebkitHtmlSelectReplacer> _selectReplacer;
    QTimer _timer;
    QMutex _mutex;

    QImage _image;

    bool _interactionModeActive;

    unsigned int _initialWidth;

    void processClickEvent(const deflect::Event& clickEvent);
    void processPressEvent(const deflect::Event& pressEvent);
    void processMoveEvent(const deflect::Event& moveEvent);
    void processReleaseEvent(const deflect::Event& releaseEvent);
    void processWheelEvent(const deflect::Event& wheelEvent);
    void processKeyPress(const deflect::Event& keyEvent);
    void processKeyRelease(const deflect::Event& keyEvent);
    void processViewSizeChange(const deflect::Event& sizeEvent);

    QWebHitTestResult performHitTest(const deflect::Event &event) const;
    QPoint getPointerPosition(const deflect::Event& event) const;
    bool isWebGLElement(const QWebElement& element) const;
    void setSize(const QSize& webpageSize);
    void recomputeZoomFactor();
};

#endif
