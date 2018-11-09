/*********************************************************************/
/* Copyright (c) 2018, EPFL/Blue Brain Project                       */
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
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#define BOOST_TEST_MODULE PixelStreamControllerTests
#include <boost/test/unit_test.hpp>

#include "DummyContent.h"

#include "control/PixelStreamController.h"
#include "scene/ContentFactory.h"
#include "scene/Window.h"

namespace
{
const int WIDTH = 512;
const int HEIGHT = 256;

inline QPointF _normalize(const QPointF& point)
{
    return QPointF{point.x() / WIDTH, point.y() / HEIGHT};
}
}

struct PixelStreamFixture
{
    PixelStreamFixture()
    {
        QObject::connect(&controller, &PixelStreamController::notify,
                         [this](const auto& evt) { event = evt; });
    }
    Window window{
        ContentFactory::getPixelStreamContent("xyz", QSize{WIDTH, HEIGHT})};
    PixelStreamController controller{window};
    deflect::Event event;

    QPointF eventPosition() const
    {
        return QPointF{event.mouseX, event.mouseY};
    }
    QPointF eventDelta() const { return QPointF{event.dx, event.dy}; }
};

BOOST_FIXTURE_TEST_CASE(tap_event, PixelStreamFixture)
{
    controller.tap(QPointF{WIDTH * 0.5, HEIGHT * 0.25}, 1);
    BOOST_CHECK(event.type == deflect::Event::EVT_CLICK);
    BOOST_CHECK_EQUAL(event.key, 1);
    BOOST_CHECK_EQUAL(eventPosition(), QPointF(0.5, 0.25));

    controller.tap(QPointF{0.0, 0.0}, 3);
    BOOST_CHECK(event.type == deflect::Event::EVT_CLICK);
    BOOST_CHECK_EQUAL(event.key, 3);
    BOOST_CHECK_EQUAL(eventPosition(), QPointF(0.0, 0.0));

    controller.tap(QPointF{WIDTH, HEIGHT}, 2);
    BOOST_CHECK(event.type == deflect::Event::EVT_CLICK);
    BOOST_CHECK_EQUAL(event.key, 2);
    BOOST_CHECK_EQUAL(eventPosition(), QPointF(1.0, 1.0));
}

BOOST_FIXTURE_TEST_CASE(pinch_event, PixelStreamFixture)
{
    const auto pos = QPointF{WIDTH * 0.5, HEIGHT * 0.25};
    const auto delta = QPointF{5.0, 3.0};
    controller.pinch(pos, delta);

    BOOST_CHECK(event.type == deflect::Event::EVT_PINCH);
    BOOST_CHECK_EQUAL(eventPosition(), QPointF(0.5, 0.25));
    BOOST_CHECK_EQUAL(eventDelta(), _normalize(delta));
}

BOOST_FIXTURE_TEST_CASE(move_event, PixelStreamFixture)
{
    const auto pos = QPointF{WIDTH * 0.5, HEIGHT * 0.25};
    const auto delta = QPointF{5.0, 3.0};
    controller.pan(pos, delta, 1);

    BOOST_CHECK(event.type == deflect::Event::EVT_MOVE);
    BOOST_CHECK_EQUAL(eventPosition(), QPointF(0.5, 0.25));
    BOOST_CHECK_EQUAL(eventDelta(), _normalize(delta));
}

BOOST_FIXTURE_TEST_CASE(pan_event, PixelStreamFixture)
{
    const auto pos = QPointF{WIDTH * 0.5, HEIGHT * 0.25};
    const auto delta = QPointF{5.0, 3.0};
    controller.pan(pos, delta, 2);

    BOOST_CHECK(event.type == deflect::Event::EVT_PAN);
    BOOST_CHECK_EQUAL(event.key, 2);
    BOOST_CHECK_EQUAL(eventPosition(), QPointF(0.5, 0.25));
    BOOST_CHECK_EQUAL(eventDelta(), _normalize(delta));
}

BOOST_FIXTURE_TEST_CASE(resize_event, PixelStreamFixture)
{
    window.setCoordinates(QRectF{QPointF(), QSizeF{100, 150}});
    BOOST_CHECK_EQUAL(event.type, deflect::Event::EVT_VIEW_SIZE_CHANGED);
    BOOST_CHECK_EQUAL(eventDelta(), QPointF(100, 150));

    window.setCoordinates(QRectF{QPointF(), QSizeF{2000, 1500}});
    BOOST_CHECK_EQUAL(eventDelta(), QPointF(2000, 1500));

    deflect::SizeHints hints;
    hints.minWidth = 200;
    hints.minHeight = 200;
    hints.maxWidth = 1000;
    hints.maxHeight = 1000;
    window.getContent().setSizeHints(hints);

    window.setCoordinates(QRectF{QPointF(), QSizeF{500, 500}});
    BOOST_CHECK_EQUAL(eventDelta(), QPointF(500, 500));

    window.setCoordinates(QRectF{QPointF(), QSizeF{100, 150}});
    BOOST_CHECK_EQUAL(eventDelta(), QPointF(200, 300));

    window.setCoordinates(QRectF{QPointF(), QSizeF{2000, 1500}});
    BOOST_CHECK_EQUAL(eventDelta(), QPointF(1000, 750));
}
