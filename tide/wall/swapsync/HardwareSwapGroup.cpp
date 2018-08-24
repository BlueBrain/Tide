/*********************************************************************/
/* Copyright (c) 2017-2018, EPFL/Blue Brain Project                  */
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

#include "HardwareSwapGroup.h"

#include <QOpenGLContext>
#if TIDE_USE_QT5X11EXTRAS
#include <QtX11Extras/QX11Info>
#endif

namespace
{
#if TIDE_USE_QT5X11EXTRAS

// The Qt WId returned by QWindow::winId() is the desired GLXDrawable since:
// WId QXcbWindow::winId() const { return m_window; }
// xcb_window_t m_window;
//
// And:
// - GLXDrawable = XID = unsigned long (= uint32_t)
// - XID === xcb_window_t = uint32_t

using GLXDrawable = uint32_t;

// Alternatively the bound drawable (window) could be retrived at runtime:
//
// using GetCurrentDrawable = GLXDrawable(QOPENGLF_APIENTRYP)();
// GetCurrentDrawable _getGetCurrentDrawable()
// {
//     auto context = _getCurrentGlContext();
//     if (auto func = context->getProcAddress("glXGetCurrentDrawable"))
//         return reinterpret_cast<GetCurrentDrawable>(func);
//     throw std::runtime_error("missing glXGetCurrentDrawable extension");
// }

using QueryMaxSwapGroups = bool(QOPENGLF_APIENTRYP)(Display*, int, GLuint*,
                                                    GLuint*);
using JoinSwapGroupFunc = bool(QOPENGLF_APIENTRYP)(Display*, GLXDrawable,
                                                   GLuint);
using BindSwapBarrierFunc = bool(QOPENGLF_APIENTRYP)(Display*, GLuint, GLuint);

QOpenGLContext* _getCurrentGlContext()
{
    if (auto context = QOpenGLContext::currentContext())
        return context;
    throw std::runtime_error("no current gl context");
}

QueryMaxSwapGroups _getQueryMaxSwapGroupsFunc()
{
    auto context = _getCurrentGlContext();
    if (auto func = context->getProcAddress("glXQueryMaxSwapGroupsNV"))
        return reinterpret_cast<QueryMaxSwapGroups>(func);
    throw std::runtime_error("missing glXQueryMaxSwapGroupsNV extension");
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

std::pair<GLuint, GLuint> _getMaxGroupAndBarrier()
{
    GLuint maxGroup = 0;
    GLuint maxBarrier = 0;

    auto queryMaxSwapGroups = _getQueryMaxSwapGroupsFunc();
    queryMaxSwapGroups(QX11Info::display(), QX11Info::appScreen(), &maxGroup,
                       &maxBarrier);

    return std::make_pair(maxGroup, maxBarrier);
}

void _checkGroup(const GLuint group)
{
    const auto maxGroup = _getMaxGroupAndBarrier().first;
    if (group > maxGroup)
    {
        throw std::runtime_error(
            std::string("Required swap group is not available: ") +
            std::to_string(group) + " >= " + std::to_string(maxGroup));
    }
}

void _checkBarrier(const GLuint barrier)
{
    const auto maxBarrier = _getMaxGroupAndBarrier().second;
    if (barrier > maxBarrier)
    {
        throw std::runtime_error(
            std::string("Required swap barrier is not available: ") +
            std::to_string(barrier) + " >= " + std::to_string(maxBarrier));
    }
}

bool _joinSwapGroup(const uint winId, const GLuint group)
{
    _checkGroup(group);

    auto joinSwapGroup = _getJoinSwapGroupFunc();
    return joinSwapGroup(QX11Info::display(), winId, group);
    throw std::runtime_error("Failed to get current drawable");
}

bool _bindSwapBarrier(const GLuint group, const GLuint barrier)
{
    _checkBarrier(barrier);

    auto bindSwapBarrier = _getBindSwapBarrierFunc();
    return bindSwapBarrier(QX11Info::display(), group, barrier);
}

#else

bool _joinSwapGroup(const uint, const GLuint)
{
    throw std::runtime_error("Unsupported platform, need QX11Info");
}

bool _bindSwapBarrier(const GLuint, const GLuint)
{
    throw std::runtime_error("Unsupported platform, need QX11Info");
}

#endif // TIDE_USE_QT5X11EXTRAS
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
