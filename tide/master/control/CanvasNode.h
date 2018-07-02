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

#ifndef CANVASNODE_H
#define CANVASNODE_H

#include "scene/ContentType.h"
#include "types.h"

/**
 * Represent a node or a leaf in the binary tree structure used by
 * AutomaticLayout
 */
class CanvasNode : public QRectF,
                   public std::enable_shared_from_this<CanvasNode>
{
public:
    using NodePtr = std::shared_ptr<CanvasNode>;

    CanvasNode(NodePtr rootPtr, NodePtr parent, NodePtr firstChild,
               NodePtr secondChild, QRectF rect);
    CanvasNode(QRectF available_space);
    CanvasNode(NodePtr rootPtr, NodePtr parent, WindowPtr window, QRectF rect);
    bool insert(WindowPtr window);
    /**
     * return true if there is an empty leaf
     */
    bool isFree() const;
    bool isRoot() const;
    bool isTerminal() const;
    void updateFocusCoordinates();
    /**
     * resize the tree, insertion should not be used after call to preview
     */
    void preview();
    qreal getOccupiedSpace();
    const QRectF availableSpace;
    NodePtr rootPtr;
    NodePtr parent;
    NodePtr firstChild;
    NodePtr secondChild;

private:
    void _update();
    qreal _getOccupiedSpace() const;
    bool previewed = false;
    void _constrainTerminalIntoRect(const QRectF& rect);
    void _constrainNodeIntoRect(const QRectF& rect);

    QRectF _rectWithoutMargins(const QRectF& rect) const;
    QRectF _rectWithoutMargins(const QRectF& rect,
                               CONTENT_TYPE content_type) const;
    bool _insertRoot(WindowPtr window);
    bool _insertTerminal(WindowPtr window);
    void _computeBoundaries(const QRectF& realSize,
                            QRectF& internalNodeBoundaries,
                            QRectF& internalFreeLeafBoundaries,
                            QRectF& externalFreeLeafBoundaries) const;
    bool _insertSecondChild(WindowPtr window);
    bool _chooseVerticalCut(const QRectF& realSize) const;
    void _setRect(QRectF newRect);
    void _constrainIntoRect(const QRectF& rect);
    WindowPtr content = NULL;
    QRectF _addMargins(WindowPtr window) const;
    QRectF _addMargins(const QRectF& rect) const;
    QRectF _addMargins(const QRectF& rect, CONTENT_TYPE type) const;
};

#endif
