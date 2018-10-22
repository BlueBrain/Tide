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

#include "PlanarController.h"

#include "utils/log.h"

namespace
{
constexpr int readTimeoutMs = 1000;
constexpr int refreshTimeMs = 60000;
}

PlanarController::PlanarController(const QString& serialport, const Type type)
    : _config{_getConfig(type)}
{
    _serial.setPortName(serialport);
    _serial.setBaudRate(_config.baudrate, QSerialPort::AllDirections);
    _serial.setDataBits(QSerialPort::Data8);
    _serial.setParity(QSerialPort::NoParity);
    _serial.setStopBits(QSerialPort::OneStop);
    _serial.setFlowControl(QSerialPort::NoFlowControl);
    if (!_serial.open(QIODevice::ReadWrite))
        throw std::runtime_error("Could not open " + serialport.toStdString());

    connect(&_serial, &QSerialPort::readyRead, [this, type]() {
        if (_serial.canReadLine())
        {
            _readingTimeoutTimer.stop();

            auto output = QString{_serial.readLine()}.trimmed();
            // TV_UR9850 returns "(0;PWR=0)"
            // Others return "DISPLAY.POWER=O" or "DISPLAY.POWER=OFF"
            if (type == Type::TV_UR9850)
                output.remove(")");

            if (output.endsWith("OFF") || output.endsWith("0"))
                _updateState(ScreenState::off);
            else if (output.endsWith("ON") || output.endsWith("1"))
                _updateState(ScreenState::on);
            else
                _updateState(ScreenState::undefined);
        }
    });

#if QT_VERSION >= 0x050800
    constexpr auto errorSignal = &QSerialPort::errorOccurred;
#else
    constexpr auto errorSignal =
        static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(
            &QSerialPort::error);
#endif
    connect(&_serial, errorSignal, this, &PlanarController::_handleError);

    _readingTimeoutTimer.setSingleShot(true);
    connect(&_readingTimeoutTimer, &QTimer::timeout, this,
            &PlanarController::_handleReadTimeout);

    connect(&_refreshTimer, &QTimer::timeout,
            [this] { checkState(ScreenStateCallback()); });
    _refreshTimer.start(refreshTimeMs);
    checkState(ScreenStateCallback());
}

ScreenState PlanarController::getState() const
{
    return _state;
}

void PlanarController::checkState(ScreenStateCallback callback)
{
    _callbacks.push_back(std::move(callback));
    _serial.write(_config.powerState);
    if (!_readingTimeoutTimer.isActive())
        _readingTimeoutTimer.start(readTimeoutMs);
}

void PlanarController::powerOn(BoolCallback callback)
{
    _serial.write(_config.powerOn);
    checkState([callback](const ScreenState state) {
        if (callback)
            callback(state == ScreenState::on);
    });
}

void PlanarController::powerOff(BoolCallback callback)
{
    _serial.write(_config.powerOff);
    checkState([callback](const ScreenState state) {
        if (callback)
            callback(state == ScreenState::off);
    });
}

void PlanarController::_updateState(const ScreenState state)
{
    const auto previousState = _state;
    _state = state;

    if (_state != previousState)
        emit powerStateChanged(_state);

    for (const auto& callback : _callbacks)
    {
        if (callback)
            callback(_state);
    }
    _callbacks.clear();
}

void PlanarController::_handleReadTimeout()
{
    _updateState(ScreenState::undefined);
}

void PlanarController::_handleError(const QSerialPort::SerialPortError error)
{
    put_log(LOG_ERROR, LOG_POWER,
            "An error (%d) occurred with serial port '%s': %s", error,
            _serial.portName().toLocal8Bit().constData(),
            _serial.errorString().toLocal8Bit().constData());
}

PlanarController::PlanarConfig PlanarController::_getConfig(const Type type)
{
    switch (type)
    {
    case Type::Matrix:
        return {9600, "OPA1DISPLAY.POWER=ON\r", "OPA1DISPLAY.POWER=OFF\r",
                "OPA1DISPLAY.POWER?\r"};
    case Type::TV_UR9850:
        return {19200, "(PWR=1)\r", "(PWR=0)\r", "(PWR?)\r"};
    case Type::TV_UR9851:
        return {19200, "DISPLAY.POWER=ON\n", "DISPLAY.POWER=OFF\n",
                "DISPLAY.POWER?\n"};
    default:
        throw std::invalid_argument("Non existing serial type");
    }
}
