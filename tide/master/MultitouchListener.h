/*********************************************************************/
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
/*                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>     */
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

#ifndef MULTITOUCHLISTENER_H
#define MULTITOUCHLISTENER_H

#include <TUIO/TuioClient.h>
#include <TUIO/TuioListener.h>

#include <QObject>
#include <QPointF>

/**
 * Listen to TUIO touch events and emit corresponding QSignals.
 */
class MultitouchListener : public QObject, public TUIO::TuioListener
{
    Q_OBJECT
    Q_DISABLE_COPY( MultitouchListener )

public:
    MultitouchListener();
    ~MultitouchListener();

    void addTuioObject( TUIO::TuioObject* ) override {}
    void updateTuioObject( TUIO::TuioObject* ) override {}
    void removeTuioObject( TUIO::TuioObject* ) override {}

    void addTuioCursor( TUIO::TuioCursor* tcur ) override;
    void updateTuioCursor( TUIO::TuioCursor* tcur ) override;
    void removeTuioCursor( TUIO::TuioCursor* tcur ) override;

    void refresh( TUIO::TuioTime ) override {}

signals:
    void touchPointAdded( int id, QPointF normalizedPos );
    void touchPointUpdated( int id, QPointF normalizedPos );
    void touchPointRemoved( int id, QPointF normalizedPos );

private:
    TUIO::TuioClient _client;
};

#endif
