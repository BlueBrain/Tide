/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
/* Copyright (c) 2013-2017, EPFL/Blue Brain Project                  */
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

#ifndef LOG_H
#define LOG_H

#include <qlogging.h>
#include <string>

#define LOG_VERBOSE 0
#define LOG_DEBUG 1
#define LOG_INFO 2
#define LOG_WARN 3
#define LOG_ERROR 4
#define LOG_FATAL 5

#define LOG_AV "AV"
#define LOG_CONTENT "CONTENT"
#define LOG_GENERAL "GENERAL"
#define LOG_MPI "MPI"
#define LOG_PDF "PDF"
#define LOG_POWER "POWER"
#define LOG_QT "QT"
#define LOG_REST "REST"
#define LOG_STREAM "STREAM"
#define LOG_TIFF "TIFF"
#define LOG_TUIO "TUIO"

extern std::string logger_id;
extern void put_log(const int level, const std::string& facility,
                    const char* format, ...);

extern void avMessageLoger(void*, int level, const char* format, va_list varg);
extern void qtMessageLogger(QtMsgType type, const QMessageLogContext& context,
                            const QString& msg);

extern void tiffMessageLoggerWarn(const char* module, const char* fmt,
                                  va_list ap);
extern void tiffMessageLoggerErr(const char* module, const char* fmt,
                                 va_list ap);

extern void tuioMessageLogger(int level, const std::string& message);

#ifdef _WIN32
#define print_log(l, facility, fmt, ...) \
    put_log(l, facility, "%s: " fmt, __FUNCTION__, ##__VA_ARGS__)
#else
#define print_log(l, facility, fmt, ...) \
    put_log(l, facility, "%s: " fmt, __PRETTY_FUNCTION__, ##__VA_ARGS__)
#endif

#endif
