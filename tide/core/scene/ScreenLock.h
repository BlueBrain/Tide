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

#ifndef SCREENLOCK_H
#define SCREENLOCK_H

#include "serialization/includes.h"
#include "types.h"

#include <QObject>

/**
 * Allow wall lock preventing unwanted streams from opening and making Web
 * Interface view-only.
 */
class ScreenLock : public QObject,
                   public std::enable_shared_from_this<ScreenLock>
{
    Q_OBJECT

    Q_PROPERTY(bool locked READ isLocked NOTIFY lockChanged)
    Q_PROPERTY(
        QStringList streamList READ getPendingStreams NOTIFY streamListChanged)

public:
    /** Create a shared ScreenLock object. */
    static ScreenLockPtr create();

    /**
     * Lock the screen. Notify about incoming streams and disallow
     * Web Interface to controll the wall
     */
    Q_INVOKABLE void lock();

    /**
     * Unlock the screen. Open pending streams and allow wall control from
     * Web Interface
     */
    Q_INVOKABLE void unlock();

    /** Check the state of the lock */
    bool isLocked() const;

    /** Accept the specified stream. */
    Q_INVOKABLE void acceptStream(QString uri);

    /** Reject the specified stream. */
    Q_INVOKABLE void rejectStream(QString uri);

    /** Request a new stream acceptance. */
    void requestStreamAcceptance(QString uri);

    /** Cancel acceptance of the specified stream. */
    void cancelStreamAcceptance(QString uri);

    /** Get the list of registered streams */
    QStringList getPendingStreams() const;

signals:
    /** Emitted when the state of the lock changes. */
    void lockChanged(bool locked);

    /** Emitted when the list of stream changes. */
    void streamListChanged();

    /** Emitted when the lock changes. */
    void modified(ScreenLockPtr lock);

    /** Emitted when a single stream is to be opened (accepted by user). */
    void streamAccepted(QString uri);

    /** Emitted when a single stream is to be closed (rejected by user). */
    void streamRejected(QString uri);

private:
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        // clang-format off
        ar & _locked;
        ar & _streams;
        // clang-format on
    }

    bool _locked = false;
    std::list<QString> _streams;

    ScreenLock() = default;
    void _add(const QString& uri);
    void _remove(const QString& uri);
    bool _isPending(const QString& uri) const;
    void _acceptAllStreams();
};

#endif
