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

#include "Surface.h"

#include "utils/compilerMacros.h"

IMPLEMENT_SERIALIZE_FOR_XML(Surface)

Surface::Surface(const size_t index, DisplayGroupPtr group)
    : _index{index}
    , _group{group}
{
    _forwardModifiedSignals();
}

Surface::Surface(const size_t index, DisplayGroupPtr group,
                 BackgroundPtr background)
    : _index{index}
    , _group{group}
    , _background{background}
{
    _forwardModifiedSignals();
}

size_t Surface::getIndex() const
{
    return _index;
}

DisplayGroup& Surface::getGroup()
{
    return *_group;
}

const DisplayGroup& Surface::getGroup() const
{
    return *_group;
}

DisplayGroupPtr Surface::getGroupPtr() const
{
    return _group;
}

Background& Surface::getBackground()
{
    return *_background;
}

const Background& Surface::getBackground() const
{
    return *_background;
}

BackgroundPtr Surface::getBackgroundPtr() const
{
    return _background;
}

ContextMenu& Surface::getContextMenu()
{
    return *_contextMenu;
}

TIDE_DISABLE_WARNING_SHADOW
void Surface::moveToThread(QThread* thread)
{
    QObject::moveToThread(thread);
    _group->moveToThread(thread);
    _background->moveToThread(thread);
    _contextMenu->moveToThread(thread);
}
TIDE_DISABLE_WARNING_SHADOW_END

void Surface::_forwardModifiedSignals()
{
    connect(_group.get(), &DisplayGroup::modified, this, &Surface::modified);
    connect(_background.get(), &Background::updated, this, &Surface::modified);
    connect(_contextMenu.get(), &ContextMenu::modified, this,
            &Surface::modified);
}
