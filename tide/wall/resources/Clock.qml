/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0

Item {
    id : clock
    width: background.width
    height: background.height

    property int hours
    property int minutes
    property int seconds
    property real shift
    property bool internationalTime: false

    property real displayedHeight: background.nominalHeight
    property real scaleFactor: displayedHeight / background.nominalHeight

    function timeChanged() {
        var date = new Date;
        hours = internationalTime ? date.getUTCHours() + Math.floor(clock.shift) : date.getHours()
        minutes = internationalTime ? date.getUTCMinutes() + ((clock.shift % 1) * 60) : date.getMinutes()
        seconds = date.getUTCSeconds();
    }

    Timer {
        interval: 100; running: true; repeat: true;
        onTriggered: clock.timeChanged()
    }

    Image {
        id: background
        source: "clock.svg"
        property real nominalWidth: 104
        property real nominalHeight: 120
        sourceSize.width: nominalWidth * scaleFactor
        sourceSize.height: nominalHeight * scaleFactor

        Image {
            id: hour
            property real rotationCenter: 6.0 * scaleFactor
            x: parent.width / 2 - width / 2
            y: parent.height / 2 - height + rotationCenter
            source: "hour.svg"
            sourceSize.width: 12 * scaleFactor
            sourceSize.height: 32 * scaleFactor
            transform: Rotation {
                id: hourRotation
                origin.x: hour.width / 2
                origin.y: hour.height - hour.rotationCenter;
                angle: (clock.hours * 30) + (clock.minutes * 0.5)
                Behavior on angle {
                    SpringAnimation { spring: 2; damping: 0.2; modulus: 360 }
                }
            }
        }

        Image {
            id: minute
            property real rotationCenter: 6.0 * scaleFactor
            x: parent.width / 2 - width / 2
            y: parent.height / 2 - height + rotationCenter
            source: "minute.svg"
            sourceSize.width: 12 * scaleFactor
            sourceSize.height: 50 * scaleFactor
            transform: Rotation {
                id: minuteRotation
                origin.x: minute.width / 2
                origin.y: minute.height - minute.rotationCenter;
                angle: clock.minutes * 6
                Behavior on angle {
                    SpringAnimation { spring: 2; damping: 0.2; modulus: 360 }
                }
            }
        }

        Image {
            id: second
            x: parent.width / 2 - width / 2
            y: parent.height / 2 - height
            source: "second.svg"
            sourceSize.width: 4 * scaleFactor
            sourceSize.height: 50 * scaleFactor
            transform: Rotation {
                id: secondRotation
                origin.x: second.width / 2
                origin.y: second.height;
                angle: clock.seconds * 6
                Behavior on angle {
                    SpringAnimation { spring: 2; damping: 0.2; modulus: 360 }
                }
            }
        }
    }
}
