/*********************************************************************/
/* Copyright (c) 2015-2018, EPFL/Blue Brain Project                  */
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

/**
 * Adapted from: http://www.open-mpi.org/community/lists/users/2008/12/7563.php
 * by Douglas Guptill <douglas.guptill@dal.ca>
 */

#include "MPINospin.h"

#include <algorithm>
#include <chrono>
#include <thread>

#define TIDE_DISABLE_MPI_NOSPIN 0 // switch for debugging purposes only

namespace
{
const size_t nsec_start = 1000;
const size_t nsec_max = 100000;

int _waitForCompletion(MPI_Request req, MPI_Status* status)
{
    auto sleep_ns = nsec_start;
    int flag = 0;
    while (!flag)
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_ns));
        sleep_ns = std::min(size_t(sleep_ns << 1), nsec_max);
        MPI_Request_get_status(req, &flag, status);
    }
    return MPI_Wait(&req, status);
}
}

int MPI_Probe_Nospin(const int source, const int tag, MPI_Comm comm,
                     MPI_Status* status)
{
#if TIDE_DISABLE_MPI_NOSPIN
    return MPI_Probe(source, tag, comm, status);
#else
    int flag = 0;
    int ret = MPI_Iprobe(source, tag, comm, &flag, status);
    if (ret != MPI_SUCCESS)
        return ret;

    timespec ts{0, nsec_start};
    while (!flag)
    {
        nanosleep(&ts, nullptr);
        ts.tv_nsec = std::min(size_t(ts.tv_nsec << 1), nsec_max);
        ret = MPI_Iprobe(source, tag, comm, &flag, status);
    }
    return ret;
#endif
}

int MPI_Send_Nospin(void* buff, const int count, MPI_Datatype datatype,
                    const int dest, const int tag, MPI_Comm comm)
{
#if TIDE_DISABLE_MPI_NOSPIN
    return MPI_Send(buff, count, datatype, dest, tag, comm);
#else
    MPI_Request req;
    const int ret = MPI_Isend(buff, count, datatype, dest, tag, comm, &req);
    if (ret != MPI_SUCCESS)
        return ret;

    return _waitForCompletion(req, MPI_STATUS_IGNORE);
#endif
}

int MPI_Recv_Nospin(void* buff, const int count, MPI_Datatype datatype,
                    const int from, const int tag, MPI_Comm comm,
                    MPI_Status* status)
{
#if TIDE_DISABLE_MPI_NOSPIN
    return MPI_Recv(buff, count, datatype, from, tag, comm, status);
#else
    MPI_Request req;
    const int ret = MPI_Irecv(buff, count, datatype, from, tag, comm, &req);
    if (ret != MPI_SUCCESS)
        return ret;

    return _waitForCompletion(req, status);
#endif
}

int MPI_Bcast_Nospin(void* buff, const int count, MPI_Datatype datatype,
                     const int root, MPI_Comm comm)
{
#if TIDE_DISABLE_MPI_NOSPIN
    MPI_Bcast(buff, count, datatype, root, comm);
#else
    MPI_Request req;
    const int ret = MPI_Ibcast(buff, count, datatype, root, comm, &req);
    if (ret != MPI_SUCCESS)
        return ret;

    return _waitForCompletion(req, MPI_STATUS_IGNORE);
#endif
}
