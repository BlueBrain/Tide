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

#include "AutomaticLayout.h"

#include "layout/CanvasNode.h"
#include "scene/DisplayGroup.h"
#include "scene/Window.h"
#include "ui.h"

namespace
{
const int maxRandomPermutations = 200;
}

AutomaticLayout::AutomaticLayout(const DisplayGroup& group)
    : _group(group)
{
}

void AutomaticLayout::updateFocusedCoord(const WindowSet& windows) const
{
    const auto availableSpace = _getAvailableSpace();
    auto sortedWindows = _sortByMaxRatio(windows);
    auto layout = std::make_unique<CanvasNode>(sortedWindows, availableSpace);

    // seed the random function, so every execution will be identical
    std::srand(0);

    // We keep the tree for which used space is maximal
    for (int i = 0; i < maxRandomPermutations; ++i)
    {
        std::random_shuffle(sortedWindows.begin(), sortedWindows.end());
        auto tree = std::make_unique<CanvasNode>(sortedWindows, availableSpace);
        if (tree->getOccupiedSpace() > layout->getOccupiedSpace())
            layout = std::move(tree);
    }
    layout->updateWindowCoordinates();
}

QRectF AutomaticLayout::_getAvailableSpace() const
{
    return ui::getFocusSurface(_group);
}

WindowPtrs AutomaticLayout::_sortByMaxRatio(const WindowSet& windows) const
{
    auto sortedWindows = std::vector<WindowPtr>{windows.begin(), windows.end()};
    std::sort(sortedWindows.begin(), sortedWindows.end(),
              [this](WindowPtr a, WindowPtr b) {
                  return _computeMaxRatio(*a) > _computeMaxRatio(*b);
              });
    return sortedWindows;
}

qreal AutomaticLayout::_computeMaxRatio(const Window& window) const
{
    const auto space = _getAvailableSpace();
    return std::max(window.width() / space.width(),
                    window.height() / space.height());
}
