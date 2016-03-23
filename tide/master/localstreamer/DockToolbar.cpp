/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#include "DockToolbar.h"

#include <QPainter>
#include <QColor>

DockToolbar::DockToolbar(const QSize size)
    : area_(0, 0, size.width(), size.height())
    , image_(size, QImage::Format_RGB32)
    , needsUpdate_(true)
{
}

DockToolbar::~DockToolbar()
{
    foreach(ToolbarButton* button, buttons_)
    {
        delete button;
    }
}

void DockToolbar::render(QImage& buffer) const
{
    QPainter painter;
    painter.begin(&buffer);

    QBrush brush;
    brush.setColor(Qt::gray);
    brush.setStyle(Qt::SolidPattern);
    painter.fillRect(area_, brush);

    int i = 0;
    foreach(ToolbarButton* button, buttons_)
    {
        drawButton(painter, *button, i++);
    }

    painter.end();
}

void DockToolbar::addButton(ToolbarButton* button)
{
    buttons_.push_back(button);
    needsUpdate_ = true;
}

QSize DockToolbar::getSize() const
{
    return area_.size();
}

const ToolbarButton* DockToolbar::getButtonAt(const QPoint& pos) const
{
    if (!area_.contains(pos))
        return 0;

    const unsigned int index = (float)pos.x() / (float)area_.width() * buttons_.size();

    if ((int)index >= buttons_.size())
        return 0;

    return buttons_.at(index);
}

const QImage& DockToolbar::getImage() const
{
    if(needsUpdate_)
    {
        render(image_);
        needsUpdate_ = false;
    }

    return image_;
}

void DockToolbar::drawButton(QPainter& painter, const ToolbarButton& button, const int index) const
{
    // Compute dimensions
    const unsigned int buttonsCount = buttons_.size();
    const unsigned int margin = area_.height() * 0.1;
    const unsigned int buttonWidth = (area_.width() - (buttonsCount+1)*margin) / buttonsCount;
    const unsigned int buttonHeight = area_.height() - 2*margin;

    const QPoint topLeft(margin + index*(buttonWidth+margin), margin);
    const QSize buttonSize(buttonWidth, buttonHeight);
    const QRect buttonArea(topLeft, buttonSize);

    // Render background
    QBrush brush;
    brush.setColor(Qt::lightGray);
    brush.setStyle(Qt::SolidPattern);
    painter.setBrush(brush); // Filling
    painter.setPen(Qt::NoPen); // Outline
    painter.drawRoundedRect(buttonArea, 5, 5);

    // Render icon
    QRect imageArea(buttonArea);
    imageArea.setWidth(imageArea.height());
    painter.drawImage(imageArea, button.icon);

    // Render caption
    const QRect textArea(imageArea.topRight() + QPoint(margin, margin),
                         buttonArea.bottomRight() - QPoint(margin, margin));
    QFont font("Arial", textArea.height());
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(textArea, button.caption, QTextOption(Qt::AlignVCenter));
}
