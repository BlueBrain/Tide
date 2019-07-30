/*********************************************************************/
/* Copyright (c) 2014-2019, EPFL/Blue Brain Project                  */
/*                          Pawel Podhajski <pawel.podhajski@epfl.ch>*/
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

#include "TouchFilter.h"

#include "utils/log.h"

#include <QDebug>
#include <QEvent>
#include <QQuickItem>
#include <QTouchEvent>

#include <cmath>

TouchFilter::TouchFilter(const QMargins& margins, const QSize& surfaceSize)
    : QObject()
    , _touchSurface(margins.left(), margins.top(),
                    surfaceSize.width() - margins.right() - margins.left(),
                    surfaceSize.height() - margins.top() - margins.bottom())
{
}

bool TouchFilter::eventFilter(QObject* object, QEvent* event)
{
    auto touchEvent = dynamic_cast<const QTouchEvent*>(event);
    if (!touchEvent)
        return false;

    auto quickitem = dynamic_cast<QQuickItem*>(object);
    if (!quickitem)
        return false;

    for (auto touchPoint : touchEvent->touchPoints())
    {
        auto globalPos = quickitem->mapToGlobal(touchPoint.pos());
        QPoint point(int(globalPos.x()), int(globalPos.y()));

        if (!_touchSurface.contains(point))
        {
            qDebug() << "Point discarded: " << point
                     << " Touch surface: " << _touchSurface;
            put_log(LOG_DEBUG, LOG_TOUCH, "Point discarded x: %i y: %i",
                    point.x(), point.y());
            return true;
        }
    }
    return false;
}
