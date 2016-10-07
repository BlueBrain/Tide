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

#include "WebkitPixelStreamer.h"

#include "log.h"
#include "scene/WebbrowserContent.h"
#include "scene/WebbrowserHistory.h"
#include "WebkitAuthenticationHelper.h"
#include "WebkitHtmlSelectReplacer.h"

#ifdef TIDE_USE_ZEROEQ
#  include "rest/RestCommand.h"
#  include "rest/RestServer.h"
#endif

#include <QKeyEvent>
#include <QWebElement>
#include <QWebFrame>
#include <QWebHistory>
#include <QWebView>

#define WEBPAGE_MIN_WIDTH      640
#define WEBPAGE_MIN_HEIGHT     512

#define WEBPAGE_DEFAULT_ZOOM   2.0

#ifdef TIDE_USE_ZEROEQ
class RestInterface
{
public:
    RestInterface()
    {
        _httpServer.get().subscribe( loadCmd );
    }
    int getPort() const
    {
        return _httpServer.getPort();
    }
    RestCommand loadCmd{ "load" };

private:
    RestServer _httpServer;
};
#endif

WebkitPixelStreamer::WebkitPixelStreamer( const QSize& webpageSize,
                                          const QString& url )
    : PixelStreamer()
    , _authenticationHelper( new WebkitAuthenticationHelper( _webView ))
    , _selectReplacer( new WebkitHtmlSelectReplacer( _webView ))
    , _initialWidth( std::max( webpageSize.width(), WEBPAGE_MIN_WIDTH ))
{
    setSize( webpageSize * WEBPAGE_DEFAULT_ZOOM );
    _webView.setZoomFactor(WEBPAGE_DEFAULT_ZOOM);

    QWebSettings* settings = _webView.settings();
    settings->setAttribute( QWebSettings::AcceleratedCompositingEnabled, true );
    settings->setAttribute( QWebSettings::JavascriptEnabled, true );
    settings->setAttribute( QWebSettings::PluginsEnabled, true );
    settings->setAttribute( QWebSettings::LocalStorageEnabled, true );
    settings->setAttribute( QWebSettings::WebGLEnabled, true );

#ifdef TIDE_USE_ZEROEQ
    _restInterface.reset( new RestInterface );
    connect( &_restInterface->loadCmd, &RestCommand::received,
             [this]( const QString uri ) { setUrl( uri ); });
#endif
    connect( &_webView, &QWebView::urlChanged, [this]()
    {
        const auto history = WebbrowserHistory{ *_webView.history( )};
        const auto port = _restInterface ? _restInterface->getPort() : 0;
        emit stateChanged( WebbrowserContent::serializeData( history, port ));
    });

    setUrl( url );

    connect(&_timer, SIGNAL(timeout()), this, SLOT(_update()));
    _timer.start(30);
}

WebkitPixelStreamer::~WebkitPixelStreamer()
{
    _timer.stop();
}

void WebkitPixelStreamer::setUrl( const QString& url )
{
    auto address = QUrl{ url };
    if( address.scheme().isEmpty( ))
        address.setScheme( "http" );

    const QMutexLocker lock( &_mutex );
    _webView.load( address );
}

const QWebView* WebkitPixelStreamer::getView() const
{
    return &_webView;
}

void WebkitPixelStreamer::processEvent(deflect::Event event_)
{
    QMutexLocker locker(&_mutex);

    switch(event_.type)
    {
    case deflect::Event::EVT_CLICK:
        processClickEvent(event_);
        break;
    case deflect::Event::EVT_PRESS:
        processPressEvent(event_);
        break;
    case deflect::Event::EVT_MOVE:
        processMoveEvent(event_);
        break;
    case deflect::Event::EVT_PINCH:
        processPinchEvent(event_);
        break;
    case deflect::Event::EVT_RELEASE:
        processReleaseEvent(event_);
        break;
    case deflect::Event::EVT_SWIPE_LEFT:
        _webView.back();
        break;
    case deflect::Event::EVT_SWIPE_RIGHT:
        _webView.forward();
        break;
    case deflect::Event::EVT_KEY_PRESS:
        processKeyPress(event_);
        break;
    case deflect::Event::EVT_KEY_RELEASE:
        processKeyRelease(event_);
        break;
    case deflect::Event::EVT_VIEW_SIZE_CHANGED:
        processViewSizeChange(event_);
        break;
    case deflect::Event::EVT_PAN:
        processMoveEvent(event_);
        break;
    default:
        break;
    }
}

void WebkitPixelStreamer::processClickEvent( const deflect::Event& clickEvent )
{
    // TODO check if this workaround for links is still needed
    const QWebHitTestResult& hitResult = performHitTest( clickEvent );
    if( !hitResult.isNull() && !hitResult.linkUrl().isEmpty( ))
        _webView.load( hitResult.linkUrl( ));
}

void WebkitPixelStreamer::processPressEvent(const deflect::Event& pressEvent)
{
    const QWebHitTestResult& hitResult = performHitTest(pressEvent);

    if(hitResult.isNull() || isWebGLElement(hitResult.element()))
    {
        _interactionModeActive = true;
    }

    const QPoint& pointerPos = getPointerPosition(pressEvent);

    QMouseEvent myEvent(QEvent::MouseButtonPress, pointerPos,
                        Qt::LeftButton, Qt::LeftButton,
                        (Qt::KeyboardModifiers)pressEvent.modifiers);

    _webView.page()->event(&myEvent);
}


void WebkitPixelStreamer::processMoveEvent(const deflect::Event& moveEvent)
{
    const QPoint& pointerPos = getPointerPosition(moveEvent);

    if( _interactionModeActive && moveEvent.key < 2 )
    {
        QMouseEvent myEvent(QEvent::MouseMove, pointerPos,
                            Qt::LeftButton, Qt::LeftButton,
                            (Qt::KeyboardModifiers)moveEvent.modifiers);

        _webView.page()->event(&myEvent);
    }
    else
    {
        QWebFrame *pFrame = _webView.page()->frameAt(pointerPos);
        if (!pFrame)
            return;

        int dx = moveEvent.dx * _webView.page()->viewportSize().width();
        int dy = moveEvent.dy * _webView.page()->viewportSize().height();

        pFrame->scroll(-dx,-dy);
    }
}

void WebkitPixelStreamer::processReleaseEvent(const deflect::Event& releaseEvent)
{
    const QPoint& pointerPos = getPointerPosition(releaseEvent);

    QMouseEvent myEvent(QEvent::MouseButtonRelease, pointerPos,
                        Qt::LeftButton, Qt::LeftButton,
                        (Qt::KeyboardModifiers)releaseEvent.modifiers);

    _webView.page()->event(&myEvent);

    _interactionModeActive = false;
}

void WebkitPixelStreamer::processPinchEvent( const deflect::Event& pinchEvent )
{
    const QWebHitTestResult& hitResult = performHitTest( pinchEvent );

    if( !hitResult.isNull() && isWebGLElement( hitResult.element( )))
    {
        const auto dx = pinchEvent.dx * size().width();
        const auto dy = pinchEvent.dy * size().height();
        const auto delta = std::copysign( std::sqrt( dx*dx + dy*dy ), dx + dy );

        QWheelEvent wheelEvent( hitResult.pos(), delta, Qt::NoButton,
                                (Qt::KeyboardModifiers)pinchEvent.modifiers,
                                Qt::Vertical );
        _webView.page()->event( &wheelEvent );
    }
}

void WebkitPixelStreamer::processKeyPress(const deflect::Event& keyEvent)
{
    QKeyEvent myEvent(QEvent::KeyPress, keyEvent.key,
                      (Qt::KeyboardModifiers)keyEvent.modifiers,
                      QString::fromStdString(keyEvent.text)
                      );
    _webView.page()->event(&myEvent);
}

void WebkitPixelStreamer::processKeyRelease(const deflect::Event& keyEvent)
{
    QKeyEvent myEvent(QEvent::KeyRelease, keyEvent.key,
                      (Qt::KeyboardModifiers)keyEvent.modifiers,
                      QString::fromStdString(keyEvent.text)
                      );
    _webView.page()->event(&myEvent);
}

void WebkitPixelStreamer::processViewSizeChange(const deflect::Event& sizeEvent)
{
    setSize( QSize((int)sizeEvent.dx, (int)sizeEvent.dy) );
    recomputeZoomFactor();
}

void WebkitPixelStreamer::setSize(const QSize& webpageSize)
{
    const QSize newSize(std::max(webpageSize.width(), WEBPAGE_MIN_WIDTH),
                        std::max(webpageSize.height(), WEBPAGE_MIN_HEIGHT));

    _webView.page()->setViewportSize( newSize );
}

void WebkitPixelStreamer::recomputeZoomFactor()
{
    _webView.setZoomFactor( qreal(size().width()) / qreal(_initialWidth) );
}

QSize WebkitPixelStreamer::size() const
{
    return _webView.page()->viewportSize();
}

void WebkitPixelStreamer::_update()
{
    QMutexLocker locker(&_mutex);

    QWebPage* page = _webView.page();
    if( !page->viewportSize().isEmpty())
    {
        if (_image.size() != page->viewportSize())
            _image = QImage( page->viewportSize(), QImage::Format_ARGB32 );

        QPainter painter( &_image );
        page->mainFrame()->render( &painter );
        painter.end();

        emit imageUpdated(_image);
    }
}

QWebHitTestResult
WebkitPixelStreamer::performHitTest(const deflect::Event& event_) const
{
    const QPoint& pointerPos = getPointerPosition(event_);
    QWebFrame *pFrame = _webView.page()->frameAt(pointerPos);
    return pFrame ? pFrame->hitTestContent(pointerPos) : QWebHitTestResult();
}

QPoint
WebkitPixelStreamer::getPointerPosition(const deflect::Event& event_) const
{
    QWebPage* page = _webView.page();

    int x = event_.mouseX * page->viewportSize().width();
    int y = event_.mouseY * page->viewportSize().height();

    x = std::max(0, std::min(x, page->viewportSize().width()-1));
    y = std::max(0, std::min(y, page->viewportSize().height()-1));

    return QPoint(x, y);
}

bool WebkitPixelStreamer::isWebGLElement(const QWebElement& element) const
{
    return element.tagName() == "CANVAS";
}
