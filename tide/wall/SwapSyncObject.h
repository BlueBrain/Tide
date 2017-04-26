/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#ifndef SWAPSYNCOBJECT_H
#define SWAPSYNCOBJECT_H

#include <functional>

/** Function to be used to synchronize the swapping. */
typedef std::function<bool(const uint64_t)> SyncFunction;

/**
 * Encapsulate an object to be swapped synchronously accross processes.
 */
template <typename T>
class SwapSyncObject
{
public:
    /** Callback function after synchronization. */
    typedef std::function<void(T)> SyncCallbackFunction;

    /** Default constructor. */
    SwapSyncObject()
        : _frontObject()
        , _backObject()
        , _version(0)
    {
    }

    /** Default value constructor. */
    SwapSyncObject(const T& defaultObject)
        : _frontObject(defaultObject)
        , _backObject(defaultObject)
        , _version(0)
    {
    }

    /** Get the front object */
    T get() const { return _frontObject; }
    /** Update the back object. */
    void update(const T& newObject)
    {
        _backObject = newObject;
        ++_version;
    }

    /** Synchronize the object. */
    bool sync(const SyncFunction& syncFunc)
    {
        assert(syncFunc);

        if (syncFunc(_version) && _frontObject != _backObject)
        {
            _swap();
            if (_callback)
                _callback(_frontObject);
            return true;
        }
        return false;
    }

    /** Set an optional function to call after swapping. */
    void setCallback(const SyncCallbackFunction& callback)
    {
        _callback = callback;
    }

private:
    SyncCallbackFunction _callback;
    T _frontObject;
    T _backObject;
    uint64_t _version;

    void _swap() { _frontObject = _backObject; }
};

#endif
