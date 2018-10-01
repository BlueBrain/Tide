/*********************************************************************/
/* Copyright (c) 2017-2018, EPFL/Blue Brain Project                  */
/*                          Nataniel Hofer <nataniel.hofer@epfl.ch>  */
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

#include "CanvasNode.h"

#include "scene/Window.h"
#include "ui.h"

#include <algorithm>

namespace
{
const qreal resizeFactorMaxForInsert = 1.3;
}

CanvasNode::CanvasNode(const WindowPtrs& windows, const QRectF& availableSpace)
    : _isRoot{true}
    , _availableSpace{availableSpace}
{
    for (const auto& window : windows)
        _insert(window);

    _constrainIntoRect(_availableSpace);
}

CanvasNode::CanvasNode(NodePtr firstChild, NodePtr secondChild,
                       const QRectF& rect)
    : _firstChild{std::move(firstChild)}
    , _secondChild{std::move(secondChild)}
{
    _setRect(rect);
}

CanvasNode::CanvasNode(WindowPtr window, const QRectF& rect)
    : _window{std::move(window)}
{
    _setRect(rect);
}

qreal CanvasNode::getOccupiedSpace() const
{
    if (_isTerminal())
        return _window ? width() * height() : 0.0;

    if (!_secondChild)
        return _firstChild->getOccupiedSpace();

    return _firstChild->getOccupiedSpace() + _secondChild->getOccupiedSpace();
}

void CanvasNode::updateWindowCoordinates() const
{
    if (!_isRoot && _isTerminal())
    {
        if (_window)
            _window->setFocusedCoordinates(_removeMargins(*this));
    }
    else
    {
        if (_firstChild)
            _firstChild->updateWindowCoordinates();
        if (_secondChild)
            _secondChild->updateWindowCoordinates();
    }
}

bool CanvasNode::_insert(WindowPtr window)
{
    if (_isRoot)
        _insertRoot(window);
    else if (_isTerminal())
        return _insertTerminal(window);
    else if (_firstChild->_insert(window) || _secondChild->_insert(window))
        return true;
    return false;
}

bool CanvasNode::_isFree() const
{
    if (_isTerminal())
        return !_window;

    return _firstChild->_isFree() && _secondChild->_isFree();
}

bool CanvasNode::_isTerminal() const
{
    return !_firstChild && !_secondChild;
}

void CanvasNode::_constrainTerminalIntoRect(const QRectF& rect)
{
    auto rectWithoutMargins = _removeMargins(rect);
    const auto scaleFactor =
        qreal(std::min(rectWithoutMargins.width() / _window->width(),
                       rectWithoutMargins.height() / _window->height()));
    rectWithoutMargins.setWidth(_window->width() * scaleFactor);
    rectWithoutMargins.setHeight(_window->height() * scaleFactor);

    auto rectWithMargins = _addMargins(rectWithoutMargins);
    rectWithMargins.moveCenter(rect.center());
    _setRect(rectWithMargins);
}

void CanvasNode::_constrainNodeIntoRect(const QRectF& rect)
{
    auto firstChildNewWidth = rect.width() * _firstChild->width() / width();
    auto firstChildNewHeight = rect.height() * _firstChild->height() / height();
    _firstChild->_constrainIntoRect(QRectF(rect.left(), rect.top(),
                                           firstChildNewWidth,
                                           firstChildNewHeight));
    if (_secondChild->top() == top())
    {
        _secondChild->_constrainIntoRect(
            QRectF(rect.left() + firstChildNewWidth, rect.top(),
                   rect.width() - firstChildNewWidth, firstChildNewHeight));
    }
    else
    {
        _secondChild->_constrainIntoRect(
            QRectF(rect.left(), rect.top() + firstChildNewHeight,
                   firstChildNewWidth, rect.height() - firstChildNewHeight));
    }
    _setRect(rect);
}

/**
 * Resize recursively the tree
 */
void CanvasNode::_constrainIntoRect(const QRectF& rect)
{
    if (_isRoot && !_secondChild)
    {
        if (_firstChild)
            _firstChild->_constrainIntoRect(rect);
        return;
    }

    if (_isTerminal())
        _constrainTerminalIntoRect(rect);
    else if (_secondChild->_isFree())
        _firstChild->_constrainIntoRect(rect);
    else
        _constrainNodeIntoRect(rect);
}

bool CanvasNode::_insertRoot(WindowPtr window)
{
    assert(_isRoot);

    if (_firstChild)
    {
        if (_firstChild->_insert(window))
            return true;

        if (!_secondChild)
            return _insertSecondChild(window);

        if (_secondChild->_insert(window))
            return true;

        // we have to create some new space
        _firstChild = std::make_unique<CanvasNode>(std::move(_firstChild),
                                                   std::move(_secondChild),
                                                   QRectF(topLeft(), size()));
        return _insertSecondChild(window);
    }

    _firstChild = std::make_unique<CanvasNode>(window, _addMargins(window));
    _setRect(_addMargins(window));
    return true;
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
    if (!_isFree())
        return false;

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

        auto internalNode =
            std::make_unique<CanvasNode>(nullptr, internalNodeBoundaries);
        internalNode->_firstChild = std::make_unique<CanvasNode>(
            window, QRectF(left(), top(), realSize.width(), realSize.height()));
        internalNode->_secondChild =
            std::make_unique<CanvasNode>(nullptr, internalFreeLeafBoundaries);
        _firstChild = std::move(internalNode);
        _secondChild =
            std::make_unique<CanvasNode>(nullptr, externalFreeleafBoundaries);
        return true;
    }

    // adding tolerance, we can resize the window
    if (realSize.width() <= resizeFactorMaxForInsert * width() &&
        realSize.height() <= resizeFactorMaxForInsert * height())
    {
        auto maxRatio = qreal(
            std::max(realSize.width() / width(), realSize.height() / height()));
        auto newWidthWindow = realSize.width() / maxRatio;
        auto newHeightWindow = realSize.height() / maxRatio;
        auto firstChildBoundaries =
            QRectF(left(), top(), newWidthWindow, newHeightWindow);
        auto freeLeafBoundaries =
            QRectF(left() + newWidthWindow, top() + newHeightWindow,
                   width() - newWidthWindow, height() - newHeightWindow);
        _firstChild = std::make_unique<CanvasNode>(
            CanvasNode(window, firstChildBoundaries));
        _secondChild = std::make_unique<CanvasNode>(
            CanvasNode(nullptr, nullptr, freeLeafBoundaries));
        return true;
    }

    return false;
}

// This method is called only by the rootNode : it creates some space
bool CanvasNode::_insertSecondChild(WindowPtr window)
{
    assert(_isRoot);

    auto realSize = _addMargins(window);
    if (_chooseVerticalCut(realSize))
    {
        if (realSize.height() > height())
        {
            // meaning we would have a to add some space
            auto newEmptySpace = std::make_unique<CanvasNode>(
                nullptr, QRectF(left(), height(), width(),
                                realSize.height() - height()));
            _firstChild =
                std::make_unique<CanvasNode>(std::move(_firstChild),
                                             std::move(newEmptySpace),
                                             QRectF(left(), top(), width(),
                                                    realSize.height()));
            _secondChild = std::make_unique<CanvasNode>(
                nullptr,
                QRectF(width(), top(), realSize.width(), realSize.height()));
            setRect(left(), top(), width() + realSize.width(),
                    realSize.height());
        }
        else
        {
            _secondChild = std::make_unique<CanvasNode>(
                nullptr, QRectF(width(), top(), realSize.width(), height()));
            setWidth(width() + realSize.width());
        }
    }
    else
    {
        if (realSize.width() > width())
        {
            auto newEmptySpace = std::make_unique<CanvasNode>(
                nullptr,
                QRectF(width(), top(), realSize.width() - width(), height()));
            _firstChild = std::make_unique<CanvasNode>(
                std::move(_firstChild), std::move(newEmptySpace),
                QRectF(left(), top(), realSize.width(), height()));
            _secondChild = std::make_unique<CanvasNode>(
                nullptr,
                QRectF(left(), height(), realSize.width(), realSize.height()));
            setRect(left(), top(), realSize.width(),
                    height() + realSize.height());
        }
        else
        {
            _secondChild = std::make_unique<CanvasNode>(
                nullptr, QRectF(left(), height(), width(), realSize.height()));
            setHeight(height() + realSize.height());
        }
    }
    return _insert(window);
}

bool CanvasNode::_chooseVerticalCut(const QRectF& realSize) const
{
    return (width() + realSize.width()) / _availableSpace.width() <
           (height() + realSize.height()) / _availableSpace.height();
}

void CanvasNode::_setRect(const QRectF& newRect)
{
    setRect(newRect.left(), newRect.top(), newRect.width(), newRect.height());
}

QRectF CanvasNode::_addMargins(const WindowPtr window) const
{
    return window->getCoordinates() + _getMargins(window->getContent());
}

QRectF CanvasNode::_addMargins(const QRectF& rect) const
{
    return rect + _getMargins(_window->getContent());
}

QRectF CanvasNode::_removeMargins(const QRectF& rect) const
{
    return rect - _getMargins(_window->getContent());
}

QMarginsF CanvasNode::_getMargins(const Content& content) const
{
    const auto spacing = ui::getMinWindowSpacing();
    return ui::getFocusedWindowControlsMargins(content) +
           QMarginsF{spacing, spacing, spacing, spacing};
}
