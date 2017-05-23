/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
/*                     Pawel Podhajski <pawel.podhajski@epfl.ch>     */
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

#include "ScreenLock.h"

void ScreenLock::lock()
{
    if (_locked)
        return;

    _locked = true;
    emit lockChanged(_locked);
    emit modified(shared_from_this());
}

void ScreenLock::unlock()
{
    if (!_locked)
        return;

    _acceptAllStreams();

    _locked = false;
    emit lockChanged(_locked);
    emit modified(shared_from_this());
}

bool ScreenLock::isLocked() const
{
    return _locked;
}

void ScreenLock::acceptStream(const QString uri)
{
    if (!_isPending(uri))
        return;

    emit streamAccepted(uri);
    _remove(uri);
}

void ScreenLock::rejectStream(const QString uri)
{
    if (!_isPending(uri))
        return;

    emit streamRejected(uri);
    _remove(uri);
}

void ScreenLock::requestStreamAcceptance(const QString uri)
{
    if (_locked)
        _add(uri);
    else
        acceptStream(uri);
}

void ScreenLock::cancelStreamAcceptance(const QString uri)
{
    _remove(uri);
}

QStringList ScreenLock::getPendingStreams() const
{
    return QStringList::fromStdList(_streams);
}

void ScreenLock::_add(const QString& uri)
{
    if (_isPending(uri))
        return;

    _streams.push_back(uri);
    emit streamListChanged();
    emit modified(shared_from_this());
}

void ScreenLock::_remove(const QString& uri)
{
    const auto iter = std::find(_streams.begin(), _streams.end(), uri);
    if (iter != _streams.end())
    {
        _streams.erase(iter);
        emit streamListChanged();
        emit modified(shared_from_this());
    }
}

bool ScreenLock::_isPending(const QString& uri) const
{
    return std::find(_streams.begin(), _streams.end(), uri) != _streams.end();
}

void ScreenLock::_acceptAllStreams()
{
    if (_streams.empty())
        return;

    for (auto stream : _streams)
        emit streamAccepted(stream);

    _streams.clear();
    emit streamListChanged();
}
