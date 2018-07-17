/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
/*                     Nataniel Hofer <nataniel.hofer@epfl.ch>       */
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

#include "CanvasNode.h"

#include "CanvasTree.h"
#include "LayoutPolicy.h"
#include "scene/Window.h"

#include <algorithm>

namespace
{
const qreal resizeFactorMaxForInsert = 1.3;
}

CanvasNode::CanvasNode(NodePtr _rootPtr, NodePtr _parent, NodePtr _firstChild,
                       NodePtr _secondChild, QRectF rect)
    : rootPtr(_rootPtr)
    , parent(_parent)
    , firstChild(_firstChild)
    , secondChild(_secondChild)
{
    _setRect(rect);
}

CanvasNode::CanvasNode(QRectF available_space)
    : availableSpace(available_space)
{
}

CanvasNode::CanvasNode(NodePtr _rootPtr, NodePtr _parent, WindowPtr window,
                       QRectF rect)
    : rootPtr(_rootPtr)
    , parent(_parent)
    , content(window)
{
    _setRect(rect);
}

bool CanvasNode::insert(WindowPtr window)
{
    if (isRoot())
    {
        _insertRoot(window);
    }
    else if (isTerminal())
    {
        return _insertTerminal(window);
    }
    else
    {
        if (firstChild->insert(window))
            return true;
        else if (secondChild->insert(window))
            return true;
    }
    return false;
}

bool CanvasNode::isFree() const
{
    if (isTerminal())
        return content == nullptr;
    else
        return firstChild->isFree() && secondChild->isFree();
}

bool CanvasNode::isRoot() const
{
    return parent == nullptr;
}

bool CanvasNode::isTerminal() const
{
    return (!firstChild && !secondChild);
}

void CanvasNode::updateFocusCoordinates()
{
    if (!previewed)
    {
        preview();
    }
    _update();
}

void CanvasNode::preview()
{
    _constrainIntoRect(availableSpace);
    previewed = true;
}

qreal CanvasNode::getOccupiedSpace()
{
    if (!previewed)
    {
        preview();
    }
    return _getOccupiedSpace();
}

qreal CanvasNode::_getOccupiedSpace() const
{
    if (isRoot())
    {
        if (firstChild)
        {
            if (secondChild)
            {
                return firstChild->_getOccupiedSpace() +
                       secondChild->_getOccupiedSpace();
            }
            else
            {
                return firstChild->_getOccupiedSpace();
            }
        }
        return 0;
    }
    else if (isTerminal())
    {
        if (content)
        {
            return width() * height();
        }
        return 0;
    }
    else
    {
        return firstChild->_getOccupiedSpace() +
               secondChild->_getOccupiedSpace();
    }
}

void CanvasNode::_update()
{
    if (!isRoot() && isTerminal())
    {
        if (content)
        {
            content->setFocusedCoordinates(_rectWithoutMargins(toRect()));
        }
    }
    else
    {
        if (firstChild)
        {
            firstChild->_update();
        }
        if (secondChild)
        {
            secondChild->_update();
        }
    }
}

QRectF CanvasNode::_rectWithoutMargins(const QRectF& rect) const
{
    return _rectWithoutMargins(rect, content->getContent().getType());
}

QRectF CanvasNode::_rectWithoutMargins(const QRectF& rect,
                                       ContentType content_type) const
{
    // take care that margins are respected
    auto rectWithoutMargins =
        QRectF(rect.left() + controlSpecifications::WINDOW_CONTROLS_MARGIN_PX +
                   controlSpecifications::WINDOW_SPACING_PX / 2,
               rect.top() + controlSpecifications::WINDOW_TITLE_HEIGHT +
                   controlSpecifications::WINDOW_SPACING_PX / 2,
               rect.width() - controlSpecifications::WINDOW_CONTROLS_MARGIN_PX -
                   controlSpecifications::WINDOW_SPACING_PX,
               rect.height() - controlSpecifications::WINDOW_SPACING_PX -
                   controlSpecifications::WINDOW_TITLE_HEIGHT);
    if (content_type == ContentType::movie)
    {
        rectWithoutMargins.setTop(rectWithoutMargins.top() +
                                  controlSpecifications::MOVIE_BAR_HEIGHT);
    }
    return rectWithoutMargins;
}

void CanvasNode::_constrainTerminalIntoRect(const QRectF& rect)
{
    auto rectWithoutMargins =
        _rectWithoutMargins(rect, content->getContentPtr()->getType());
    auto scaleFactor =
        qreal(std::min(rectWithoutMargins.width() / content->width(),
                       rectWithoutMargins.height() / content->height()));
    auto newWidth = content->width() * scaleFactor;
    auto newHeight = content->height() * scaleFactor;

    rectWithoutMargins.setWidth(newWidth);
    rectWithoutMargins.setHeight(newHeight);
    auto rectWithMargins = _addMargins(rectWithoutMargins);
    rectWithMargins.moveCenter(rect.center());
    setRect(rectWithMargins.left(), rectWithMargins.top(),
            rectWithMargins.width(), rectWithMargins.height());
}

void CanvasNode::_constrainNodeIntoRect(const QRectF& rect)
{
    auto firstChildNewWidth = rect.width() * firstChild->width() / width();
    auto firstChildNewHeight = rect.height() * firstChild->height() / height();
    firstChild->_constrainIntoRect(QRectF(rect.left(), rect.top(),
                                          firstChildNewWidth,
                                          firstChildNewHeight));
    if (secondChild->top() == top())
    {
        secondChild->_constrainIntoRect(
            QRectF(rect.left() + firstChildNewWidth, rect.top(),
                   rect.width() - firstChildNewWidth, firstChildNewHeight));
    }
    else
    {
        secondChild->_constrainIntoRect(
            QRectF(rect.left(), rect.top() + firstChildNewHeight,
                   firstChildNewWidth, rect.height() - firstChildNewHeight));
    }
    setRect(rect.left(), rect.top(), rect.width(), rect.height());
}

/**
 * Resize recursively the tree
 */
void CanvasNode::_constrainIntoRect(const QRectF& rect)
{
    if (isRoot() && !secondChild)
    {
        if (!firstChild)
        {
            return;
        }
        firstChild->_constrainIntoRect(rect);
        return;
    }
    else if (isTerminal())
    {
        _constrainTerminalIntoRect(rect);
    }
    else if (secondChild->isFree())
    {
        firstChild->_constrainIntoRect(rect);
    }
    else
    {
        _constrainNodeIntoRect(rect);
    }
}

bool CanvasNode::_insertRoot(WindowPtr window)
{
    if (firstChild)
    {
        if (firstChild->insert(window))
            return true;
        else if (secondChild)
        {
            if (secondChild->insert(window))
                return true;
            else
            {
                // we have to create some new space
                auto newNodePtr =
                    std::make_shared<CanvasNode>(rootPtr, rootPtr, firstChild,
                                                 secondChild,
                                                 QRectF(topLeft(), size()));
                firstChild = newNodePtr;
                return _insertSecondChild(window);
            }
        }
        else
        {
            return _insertSecondChild(window);
        }
    }
    else
    {
        firstChild = std::make_shared<CanvasNode>(rootPtr, rootPtr, window,
                                                  _addMargins(window));
        _setRect(_addMargins(window));
        return true;
    }
    return false;
}

void CanvasNode::_computeBoundaries(const QRectF& realSize,
                                    QRectF& internalNodeBoundaries,
                                    QRectF& internalFreeLeafBoundaries,
                                    QRectF& externalFreeLeafBoundaries) const
{
    if (realSize.width() / width() > realSize.height() / height())
    {
        // horizontal cut
        internalNodeBoundaries.setRect(left(), top(), width(),
                                       realSize.height());
        internalFreeLeafBoundaries.setRect(left() + realSize.width(), top(),
                                           width() - realSize.width(),
                                           realSize.height());
        externalFreeLeafBoundaries.setRect(left(), top() + realSize.height(),
                                           width(),
                                           height() - realSize.height());
    }
    else
    {
        // vertical cut
        internalNodeBoundaries.setRect(left(), top(), realSize.width(),
                                       height());
        internalFreeLeafBoundaries.setRect(left(), top() + realSize.height(),
                                           realSize.width(),
                                           height() - realSize.height());
        externalFreeLeafBoundaries.setRect(left() + realSize.height(), top(),
                                           width() - realSize.width(),
                                           height());
    }
}

/**
 * insert a window in an empty leaf, changes the structure of the tree
 */
bool CanvasNode::_insertTerminal(WindowPtr window)
{
    if (!isFree())
    {
        return false;
    }
    auto realSize = _addMargins(window);
    if (realSize.width() <= width() && realSize.height() <= height())
    {
        // separate depending on ratio (vertical or horizontal cut
        QRectF internalNodeBoundaries;
        QRectF internalFreeLeafBoundaries;
        QRectF externalFreeleafBoundaries;
        _computeBoundaries(realSize, internalNodeBoundaries,
                           internalFreeLeafBoundaries,
                           externalFreeleafBoundaries);

        auto thisPtr = shared_from_this();
        auto internalNodePtr =
            std::make_shared<CanvasNode>(rootPtr, thisPtr, nullptr, nullptr,
                                         internalNodeBoundaries);
        auto firstChildPtr =
            std::make_shared<CanvasNode>(rootPtr, internalNodePtr, window,
                                         QRectF(left(), top(), realSize.width(),
                                                realSize.height()));
        auto secondChildPtr =
            std::make_shared<CanvasNode>(rootPtr, internalNodePtr, nullptr,
                                         internalFreeLeafBoundaries);
        internalNodePtr->firstChild = firstChildPtr;
        internalNodePtr->secondChild = secondChildPtr;
        auto externalFreeLeafPtr =
            std::make_shared<CanvasNode>(rootPtr, thisPtr, nullptr,
                                         externalFreeleafBoundaries);
        firstChild = internalNodePtr;
        secondChild = externalFreeLeafPtr;
        return true;
    }
    // adding tolerance, we can resize the window
    else if (realSize.width() <= resizeFactorMaxForInsert * width() &&
             realSize.height() <= resizeFactorMaxForInsert * height())
    {
        auto maxRatio = qreal(
            std::max(realSize.width() / width(), realSize.height() / height()));
        auto thisPtr = shared_from_this();
        auto newWidthWindow = realSize.width() / maxRatio;
        auto newHeightWindow = realSize.height() / maxRatio;
        auto firstChildBoundaries =
            QRectF(left(), top(), newWidthWindow, newHeightWindow);
        auto freeLeafBoundaries =
            QRectF(left() + newWidthWindow, top() + newHeightWindow,
                   width() - newWidthWindow, height() - newHeightWindow);
        firstChild = std::make_shared<CanvasNode>(
            CanvasNode(rootPtr, thisPtr, window, firstChildBoundaries));
        secondChild = std::make_shared<CanvasNode>(
            CanvasNode(rootPtr, thisPtr, NULL, NULL, freeLeafBoundaries));
        return true;
    }
    return false;
}

// This method is called only by the rootNode : it creates some space
bool CanvasNode::_insertSecondChild(WindowPtr window)
{
    auto realSize = _addMargins(window);
    if (_chooseVerticalCut(realSize))
    {
        if (realSize.height() > height())
        {
            // meaning we would have a to add some space
            auto newEmptySpace =
                std::make_shared<CanvasNode>(rootPtr, nullptr, nullptr,
                                             QRectF(left(), height(), width(),
                                                    realSize.height() -
                                                        height()));
            auto newFirstChildNode =
                std::make_shared<CanvasNode>(rootPtr, rootPtr, firstChild,
                                             newEmptySpace,
                                             QRectF(left(), top(), width(),
                                                    realSize.height()));
            firstChild = newFirstChildNode;
            newFirstChildNode->firstChild->parent = newFirstChildNode;
            secondChild = std::make_shared<CanvasNode>(
                rootPtr, rootPtr, nullptr,
                QRectF(width(), top(), realSize.width(), realSize.height()));
            setRect(left(), top(), width() + realSize.width(),
                    realSize.height());
        }
        else
        {
            secondChild = std::make_shared<CanvasNode>(
                rootPtr, rootPtr, nullptr,
                QRectF(width(), top(), realSize.width(), height()));
            setWidth(width() + realSize.width());
        }
    }
    else
    {
        if (realSize.width() > width())
        {
            auto newEmptySpace = std::make_shared<CanvasNode>(
                rootPtr, nullptr, nullptr,
                QRectF(width(), top(), realSize.width() - width(), height()));
            auto newFirstChildNode = std::make_shared<CanvasNode>(
                rootPtr, rootPtr, firstChild, newEmptySpace,
                QRectF(left(), top(), realSize.width(), height()));
            firstChild = newFirstChildNode;
            newFirstChildNode->firstChild->parent = newFirstChildNode;
            secondChild = std::make_shared<CanvasNode>(
                rootPtr, rootPtr, nullptr,
                QRectF(left(), height(), realSize.width(), realSize.height()));
            setRect(left(), top(), realSize.width(),
                    height() + realSize.height());
        }
        else
        {
            secondChild =
                std::make_shared<CanvasNode>(rootPtr, rootPtr, nullptr,
                                             QRectF(left(), height(), width(),
                                                    realSize.height()));
            setHeight(height() + realSize.height());
        }
    }
    return insert(window);
}

bool CanvasNode::_chooseVerticalCut(const QRectF& realSize) const
{
    return (width() + realSize.width()) / availableSpace.width() <
           (height() + realSize.height()) / availableSpace.height();
}

void CanvasNode::_setRect(QRectF newRect)
{
    setRect(newRect.left(), newRect.top(), newRect.width(), newRect.height());
}

QRectF CanvasNode::_addMargins(const WindowPtr window) const
{
    return _addMargins(QRectF(window->x(), window->y(), window->width(),
                              window->height()),
                       window->getContent().getType());
}

QRectF CanvasNode::_addMargins(const QRectF& rect) const
{
    return _addMargins(rect, content->getContentPtr()->getType());
}

QRectF CanvasNode::_addMargins(const QRectF& rect, ContentType type) const
{
    auto rectWithMargins = rect.toRect();
    rectWithMargins.setTop(rectWithMargins.top() -
                           controlSpecifications::WINDOW_SPACING_PX / 2 -
                           controlSpecifications::WINDOW_TITLE_HEIGHT);
    rectWithMargins.setBottom(rectWithMargins.bottom() +
                              controlSpecifications::WINDOW_SPACING_PX / 2);
    rectWithMargins.setLeft(rectWithMargins.left() -
                            controlSpecifications::WINDOW_CONTROLS_MARGIN_PX -
                            controlSpecifications::WINDOW_SPACING_PX / 2);
    rectWithMargins.setRight(rectWithMargins.right() +
                             controlSpecifications::WINDOW_SPACING_PX / 2);
    if (type == ContentType::movie)
    {
        rectWithMargins.setTop(rectWithMargins.top() -
                               controlSpecifications::MOVIE_BAR_HEIGHT);
    }
    return rectWithMargins;
}
