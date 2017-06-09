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

#include "AutomaticLayout.h"

#include "CanvasTree.h"
#include "control/ContentWindowController.h"
#include "scene/ContentType.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"
#include "types.h"

namespace
{
const int maxRandomPermutations = 200;
}

AutomaticLayout::AutomaticLayout(const DisplayGroup& group)
    : LayoutPolicy(group)
{
}

qreal AutomaticLayout::_computeMaxRatio(ContentWindowPtr window) const
{
    return std::max(window->width() / _getAvailableSpace().width(),
                    window->height() / _getAvailableSpace().height());
}

QRectF AutomaticLayout::getFocusedCoord(const ContentWindow& window) const
{
    return _getFocusedCoord(window, _group.getFocusedWindows());
}

void AutomaticLayout::updateFocusedCoord(const ContentWindowSet& windows) const
{
    auto sortedWindows = _sortByMaxRatio(windows);
    auto layoutTree = CanvasTree(sortedWindows, _getAvailableSpace());
    auto maxOccupiedSpace = layoutTree.getOccupiedSpace();
    // seed the random function, so every execution will be identical
    std::srand(0);
    for (int i = 0; i < maxRandomPermutations; ++i)
    {
        std::random_shuffle(sortedWindows.begin(), sortedWindows.end());
        auto currentTree = CanvasTree(sortedWindows, _getAvailableSpace());
        if (currentTree.getOccupiedSpace() > maxOccupiedSpace)
        {
            layoutTree = currentTree;
            maxOccupiedSpace = currentTree.getOccupiedSpace();
        }
    }
    // We keep the tree for which used space is maximal
    layoutTree.updateFocusCoordinates();
}

qreal AutomaticLayout::_getTotalArea(const ContentWindowSet& windows) const
{
    auto areaCount = qreal(0.0);
    for (const auto& window : windows)
    {
        const auto preferredDimensions =
            window->getContentPtr()->getPreferredDimensions();
        areaCount += preferredDimensions.width() * preferredDimensions.height();
    }
    return areaCount;
}

QRectF AutomaticLayout::_getFocusedCoord(const ContentWindow& window,
                                         const ContentWindowSet& windows) const
{
    updateFocusedCoord(windows);
    return window.getCoordinates();
}

ContentWindowPtrs AutomaticLayout::_sortByMaxRatio(
    const ContentWindowSet& windows) const
{
    std::vector<ContentWindowPtr> windowVec;
    for (auto& window : windows)
    {
        windowVec.push_back(window);
    }
    std::sort(windowVec.begin(), windowVec.end(),
              [this](ContentWindowPtr a, ContentWindowPtr b) {
                  return _computeMaxRatio(a) > _computeMaxRatio(b);
              });
    return windowVec;
}
