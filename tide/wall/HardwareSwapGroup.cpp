/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
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

#include "HardwareSwapGroup.h"

#include <QOpenGLContext>
#if TIDE_USE_QT5X11EXTRAS
#include <QtX11Extras/QX11Info>
#endif

namespace
{
using JoinSwapGroupFunc = bool(QOPENGLF_APIENTRYP)(Display*, uint, GLuint);
using BindSwapBarrierFunc = bool(QOPENGLF_APIENTRYP)(Display*, GLuint, GLuint);

QOpenGLContext* _getCurrentGlContext()
{
    if (auto context = QOpenGLContext::currentContext())
        return context;
    throw std::runtime_error("no current gl context");
}

JoinSwapGroupFunc _getJoinSwapGroupFunc()
{
    auto context = _getCurrentGlContext();
    if (auto func = context->getProcAddress("glXJoinSwapGroupNV"))
        return reinterpret_cast<JoinSwapGroupFunc>(func);
    throw std::runtime_error("missing glXJoinSwapGroupNV extension");
}

BindSwapBarrierFunc _getBindSwapBarrierFunc()
{
    auto context = _getCurrentGlContext();
    if (auto func = context->getProcAddress("glXBindSwapBarrierNV"))
        return reinterpret_cast<BindSwapBarrierFunc>(func);
    throw std::runtime_error("missing glXBindSwapBarrierNV extension");
}

bool _joinSwapGroup(const uint winID, const GLuint group)
{
#if TIDE_USE_QT5X11EXTRAS
    auto joinSwapGroup = _getJoinSwapGroupFunc();
    return joinSwapGroup(QX11Info::display(), winID, group);
#else
    throw std::runtime_error("Unsupported platform, need QX11Info");
#endif
}

bool _bindSwapBarrier(const GLuint group, const GLuint barrier)
{
#if TIDE_USE_QT5X11EXTRAS
    auto bindSwapBarrier = _getBindSwapBarrierFunc();
    return bindSwapBarrier(QX11Info::display(), group, barrier);
#else
    throw std::runtime_error("Unsupported platform, need QX11Info");
#endif
}
}

HardwareSwapGroup::HardwareSwapGroup(const int id)
    : _id{id}
{
}

void HardwareSwapGroup::add(const QWindow& window)
{
    if (!_joinSwapGroup(window.winId(), _id))
        throw std::runtime_error("Failed to join swap group");
    ++_size;
}

void HardwareSwapGroup::remove(const QWindow& window)
{
    if (!_joinSwapGroup(window.winId(), 0))
        throw std::runtime_error("Failed to leave swap group");
    --_size;
}

uint HardwareSwapGroup::size() const
{
    return _size;
}

void HardwareSwapGroup::join(const int barrier)
{
    if (!_bindSwapBarrier(_id, barrier))
        throw std::runtime_error("Failed to join swap barrier");
}

void HardwareSwapGroup::leaveBarrier()
{
    if (!_bindSwapBarrier(_id, 0))
        throw std::runtime_error("Failed to leave swap barrier");
}
