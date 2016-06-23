/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
/* Copyright (c) 2013-2016, EPFL/Blue Brain Project                  */
/*                     Raphael.Dumusc@epfl.ch                        */
/*                     Daniel.Nachbaur@epfl.ch                       */
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

#include "log.h"

#include <stdarg.h>
#include <iostream>
#include <sstream>

#include <QByteArray>
#include <QString>

#define MAX_LOG_LENGTH 1024

std::string logger_id = "";

void put_log( const int level, const char* format, ... )
{
    if( level < LOG_THRESHOLD )
        return;

    char log_string[MAX_LOG_LENGTH];

    va_list ap;
    va_start( ap, format );
    vsnprintf( log_string, MAX_LOG_LENGTH, format, ap );
    va_end( ap );

    std::stringstream message;
    if( !logger_id.empty( ))
        message << "{" << logger_id << "} ";
    message << log_string;

    if( level < LOG_ERROR )
        std::cerr << message.str() << std::endl;
    else
        std::cout << message.str() << std::endl;
}

void qtMessageLogger( const QtMsgType type,
                      const QMessageLogContext& context,
                      const QString& message )
{
    const QByteArray msg = message.toLocal8Bit();
    QByteArray ctx;
    if( context.file && context.line && context.function  )
        ctx = QString( " (%1:%2, %3)" ).arg( context.file ).arg(
                  context.line ).arg( context.function ).toLocal8Bit();

    switch( type )
    {
    case QtDebugMsg:
        put_log( LOG_DEBUG, "qDebug: %s%s", msg.constData(), ctx.constData( ));
        break;
#if QT_VERSION >= 0x050500
    case QtInfoMsg:
        put_log( LOG_INFO, "qInfo: %s%s", msg.constData(), ctx.constData( ));
        break;
#endif
    case QtWarningMsg:
        put_log( LOG_WARN, "qWarning: %s%s", msg.constData(), ctx.constData( ));
        break;
    case QtCriticalMsg:
        put_log( LOG_ERROR, "qCritical: %s%s", msg.constData(),
                 ctx.constData( ));
        break;
    case QtFatalMsg:
        put_log( LOG_FATAL, "qFatal: %s%s", msg.constData(), ctx.constData( ));
        abort();
    default:
        put_log( LOG_WARN, "qMsgTypeUndef: %s%s", msg.constData(),
                 ctx.constData( ));
        break;
    }
}
