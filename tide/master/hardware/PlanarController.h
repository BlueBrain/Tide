/*********************************************************************/
/* Copyright (c) 2017-2018, EPFL/Blue Brain Project                  */
/*                          Pawel Podhajski <pawel.podhajski@epfl.ch>*/
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
        TV_UR9850,
        TV_UR9851
    };

    /**
     * Construct Planar equipment controller.
     * @param serialport the serial port used to connect to Quad Controller.
     * @param type the type of controller.
     * @throw std::runtime_error if the port is already in use or a connection
     *        issue occured.
     */
    PlanarController(const QString& serialport, const Type type);

    /** @copydoc ScreenController::getState */
    ScreenState getState() const final;

    /** @copydoc ScreenController::checkState */
    void checkState(ScreenStateCallback callback) final;

    /** @copydoc ScreenController::powerOn */
    void powerOn(BoolCallback callback) final;

    /** @copydoc ScreenController::powerOff */
    void powerOff(BoolCallback callback) final;

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
    QTimer _refreshTimer;
    QTimer _readingTimeoutTimer;
    std::vector<ScreenStateCallback> _callbacks;

    void _updateState(ScreenState state);
    void _handleReadTimeout();
    void _handleError(const QSerialPort::SerialPortError error);

    static PlanarConfig _getConfig(Type type);
};

#endif
