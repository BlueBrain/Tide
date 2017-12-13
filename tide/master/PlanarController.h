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

#ifndef PLANARCONTROLLER_H
#define PLANARCONTROLLER_H

#include "ScreenController.h"
#include "types.h"

#include <QObject>
#include <QSerialPort>
#include <QTimer>

/**
 * Allow control of Planar device over serial connection.
 */
class PlanarController : public ScreenController
{
    Q_OBJECT

public:
    /**
     * The type of the display's power controller interface.
     */
    enum class Type
    {
        Matrix,
        TV
    };

    /**
     * Construct Planar equipment controller.
     * @param serialport the serial port used to connect to Quad Controller.
     * @param type the type of controller.
     * @throw std::runtime_error if the port is already in use or a connection
     *        issue occured.
     */
    PlanarController(const QString& serialport, const Type type);

    /** Get the power state of Planar displays. */
    ScreenState getState() const final;

    /** Refresh the power state of Planar displays */
    void checkPowerState() final;

    /** Power on the displays. */
    bool powerOn() final;

    /** Power off the displays. */
    bool powerOff() final;

private:
    struct PlanarConfig
    {
        int baudrate;
        const char* powerOn;
        const char* powerOff;
        const char* powerState;
    };
    PlanarConfig _config;
    ScreenState _state;
    QSerialPort _serial;
    QTimer _timer;

    PlanarConfig _getConfig(const PlanarController::Type type) const;
};

#endif
