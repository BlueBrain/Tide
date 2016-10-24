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

#ifndef ADDRESSBAR_H
#define ADDRESSBAR_H

#include "serialization/includes.h"

#include <QObject>

/**
 * Serializable address bar state, for use in a web browser window.
 */
class AddressBar : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( AddressBar )

    Q_PROPERTY( bool focused READ isFocused WRITE setFocused
                NOTIFY focusedChanged )
    Q_PROPERTY( int cursorPosition READ getCursorPosition
                WRITE setCursorPosition NOTIFY cursorPositionChanged )
    Q_PROPERTY( int selectionStart READ getSelectionStart
                WRITE setSelectionStart NOTIFY selectionStartChanged )
    Q_PROPERTY( int selectionEnd READ getSelectionEnd
                WRITE setSelectionEnd NOTIFY selectionEndChanged )
    Q_PROPERTY( QString url READ getUrl WRITE setUrl NOTIFY urlChanged )

public:
    /** Constructor */
    explicit AddressBar( QObject* parentObject = nullptr );

    /** @return true if the address bar has focus (capture keyboard events). */
    bool isFocused() const;

    /** Give the keyboard focus to the address bar. */
    void setFocused( bool set );

    /** Get the position (index) of the cursor. */
    int getCursorPosition() const;

    /** Set the position (index) of the cursor. */
    void setCursorPosition( int index );

    /** Get the start position of the selection in the address bar. */
    int getSelectionStart() const;

    /** Set the start position of the selection in the address bar. */
    void setSelectionStart( int pos );

    /** Get the end position of the selection in the address bar. */
    int getSelectionEnd() const;

    /** Set the end position of the selection in the address bar. */
    void setSelectionEnd( int pos );

    /** Get the url in the address bar. */
    QString getUrl() const;

    /** Set the url in the address bar. */
    void setUrl( QString url );

signals:
    /** @name QProperty notifiers */
    //@{
    void focusedChanged();
    void cursorPositionChanged();
    void selectionStartChanged();
    void selectionEndChanged();
    void urlChanged();
    //@}

    void modified();

private:
    friend class boost::serialization::access;

    /** Serialize for sending to Wall applications. */
    template< class Archive >
    void serialize( Archive & ar, const unsigned int /*version*/ )
    {
        ar & _focused;
        ar & _cursorPosition;
        ar & _selectionStart;
        ar & _selectionEnd;
        ar & _url;
    }

    /** State of the address bar on master shared with the wall processes. */
    bool _focused = false;
    int _cursorPosition = 0;
    int _selectionStart = 0;
    int _selectionEnd = 0;
    QString _url;
};

#endif
