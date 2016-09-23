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

#ifndef PROCESSFORKER_H
#define PROCESSFORKER_H

#include "types.h"

#include <map>
#include <QStringList>

class QProcess;

/**
 * Run as a separate MPI process, listening to commands to fork new executables.
 *
 * This is required by a bug observed on RHEL 6 with openmpi (any version),
 * where a call to fork() from the master application has a high risk of causing
 * a segfault in malloc linked to multithreaded the MPI implementation.
 * Executing the fork() in a standalone process without multiple threads
 * works around the issue.
 *
 * Due to an incompatibility between QProcess and MPI(*), we must start the
 * processes DETACHED.
 * In theory they might stay alive after the main application has exited.
 * In practice, this doesn't happen because the processes exit when their
 * associated deflect::Stream is closed.
 *
 * (*) MPI captures the SIGCHLD that QProcess relies on to detect that the
 * process has finished. Thus, the call to waitForFinished() blocks forever in
 * QProcess destructor.
 */
class ProcessForker
{
public:
    /**
     * Constructor
     * @param mpiChannel to receive commands from the master application
     */
    explicit ProcessForker( MPIChannelPtr mpiChannel );

    /** Process MPI commands until a quit message is received. */
    void run();

private:
    MPIChannelPtr _mpiChannel;
    bool _processMessages;

    typedef std::map< QString, QProcess* > Processes;
    Processes _processes;

    void _launch( const QString& command, const QString& workingDir,
                  const QStringList& env );
};

#endif
