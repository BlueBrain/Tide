/*********************************************************************/
/* Copyright (c) 2016, EPFL/Blue Brain Project                       */
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
/*    THIS  SOFTWARE  IS  PROVIDED  BY  THE  ECOLE  POLYTECHNIQUE    */
/*    FEDERALE DE LAUSANNE  ''AS IS''  AND ANY EXPRESS OR IMPLIED    */
/*    WARRANTIES, INCLUDING, BUT  NOT  LIMITED  TO,  THE  IMPLIED    */
/*    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR    */
/*    PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  ECOLE    */
/*    POLYTECHNIQUE  FEDERALE  DE  LAUSANNE  OR  CONTRIBUTORS  BE    */
/*    LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,    */
/*    EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT    */
/*    LIMITED TO,  PROCUREMENT  OF  SUBSTITUTE GOODS OR SERVICES;    */
/*    LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS INTERRUPTION)    */
/*    HOWEVER CAUSED AND  ON ANY THEORY OF LIABILITY,  WHETHER IN    */
/*    CONTRACT, STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE    */
/*    OR OTHERWISE) ARISING  IN ANY WAY  OUT OF  THE USE OF  THIS    */
/*    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#ifndef KEYBOARDSTATE_H
#define KEYBOARDSTATE_H

#include "serialization/includes.h"

#include <QObject>

/**
 * The state of the virtual keyboard, distributed by master to wall processes.
 */
class KeyboardState : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(KeyboardState)

    Q_PROPERTY(bool visible READ isVisible NOTIFY visibleChanged)
    Q_PROPERTY(bool shift READ getShiftActive NOTIFY shiftActiveChanged)
    Q_PROPERTY(bool symbols READ getSymbolsActive NOTIFY symbolsActiveChanged)
    Q_PROPERTY(int activeKeyId READ getActiveKeyId NOTIFY activeKeyIdChanged)

public:
    explicit KeyboardState(QObject* parent = 0);

    /** @return true if the keyboard is visible. */
    bool isVisible() const;

    /** @return true if the keybard shift key is active. */
    bool getShiftActive() const;

    /** @return true if the keyboard symbols key is active. */
    bool getSymbolsActive() const;

    /** @return the identifier of the currently active key (if any) else -1. */
    int getActiveKeyId() const;

    /** Set the visibility of the keyboard. */
    void setVisible(bool visible);

    /** (De)Activate the shift key. */
    void setShiftActive(bool state);

    /** (De)Activate the symbols key. */
    void setSymbolsActive(bool state);

    /** Set the identifier of the active key. Use -1 if no key is active. */
    void setActiveKeyId(int keyId);

signals:
    /** @name QProperty notifiers */
    //@{
    void visibleChanged(bool visible);
    void shiftActiveChanged(bool state);
    void symbolsActiveChanged(bool state);
    void activeKeyIdChanged(int keyId);
    //@}

    /** Emitted whenever any field has been modified. */
    void modified();

private:
    friend class boost::serialization::access;

    /** Serialize for sending to Wall applications. */
    template <class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        // clang-format off
        ar & _visible;
        ar & _shiftActive;
        ar & _symbolsActive;
        ar & _activeKeyId;
        // clang-format on
    }

    bool _visible = false;
    bool _shiftActive = false;
    bool _symbolsActive = false;
    int _activeKeyId = -1;
};

#endif
