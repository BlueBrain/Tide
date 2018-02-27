/*********************************************************************/
/* Copyright (c) 2014-2018, EPFL/Blue Brain Project                  */
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

#include "TestPattern.h"

#include "WallConfiguration.h"

#include <QFont>
#include <QPainter>
#include <QPen>

#define FONT_SIZE 24
#define LINE_WIDTH 10
#define TEXT_POS_X 50

namespace
{
std::string to_string(const deflect::View view)
{
    switch (view)
    {
    case deflect::View::mono:
        return "mono";
    case deflect::View::left_eye:
        return "left_eye";
    case deflect::View::right_eye:
        return "right_eye";
    case deflect::View::side_by_side:
        return "side_by_side";
    default:
        throw std::logic_error("unknown deflect::View");
    }
}

std::string to_string(const bool value)
{
    return value ? "true" : "false";
}
}

TestPattern::TestPattern(const WallConfiguration& config,
                         const SurfaceConfig& surface, const Screen& screen,
                         QQuickItem& parent_)
    : QQuickPaintedItem(&parent_)
    , _wallSize(surface.getTotalSize())
{
    setVisible(false);
    setSize(_wallSize);

    const auto fullsceen = to_string(screen.fullscreen);
    const auto stereoView = to_string(screen.stereoMode);

    _windowRect = surface.getScreenRect(screen.globalIndex);

    _labels.push_back(QString("Surface: %1").arg(screen.surfaceIndex));
    _labels.push_back(QString("Rank: %1").arg(config.processIndex));
    _labels.push_back(QString("Host: %1").arg(config.host));
    _labels.push_back(QString("Display: %1").arg(screen.display));
    _labels.push_back(QString("Screen global index: (%1,%2)")
                          .arg(screen.globalIndex.x())
                          .arg(screen.globalIndex.y()));
    _labels.push_back(QString("Resolution: %1 x %2")
                          .arg(surface.getScreenWidth())
                          .arg(surface.getScreenHeight()));
    _labels.push_back(QString("Fullscreen: %1").arg(fullsceen.c_str()));
    _labels.push_back(QString("Stereo view: %1").arg(stereoView.c_str()));
}

void TestPattern::paint(QPainter* painter)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->fillRect(_windowRect, Qt::black);
    renderCrossPattern(painter);
    renderLabels(painter);
}

void TestPattern::renderCrossPattern(QPainter* painter)
{
    const qreal h = _wallSize.height();
    const qreal w = _wallSize.width();

    QPen pen;
    pen.setWidth(LINE_WIDTH);

    for (qreal y_ = -1.0 * h; y_ <= 2.0 * h; y_ += 0.1 * h)
    {
        const qreal hue = (y_ + h) / (3.0 * h);
        pen.setColor(QColor::fromHsvF(hue, 1.0, 1.0));
        painter->setPen(pen);
        painter->drawLine(QPointF(0.0, y_), QPointF(w, y_ + h));
        painter->drawLine(QPointF(0.0, y_), QPointF(w, y_ - h));
    }
}

void TestPattern::renderLabels(QPainter* painter)
{
    const auto offset = _windowRect.topLeft();

    QFont textFont;
    textFont.setPixelSize(FONT_SIZE);
    painter->setFont(textFont);
    painter->setPen(QColor(Qt::white));

    unsigned int pos = 0;
    for (const auto& label : _labels)
        painter->drawText(QPoint(TEXT_POS_X, ++pos * FONT_SIZE) + offset,
                          label);
}
