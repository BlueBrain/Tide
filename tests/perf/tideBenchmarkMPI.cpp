/*********************************************************************/
/* Copyright (c) 2014-2017, EPFL/Blue Brain Project                  */
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

#include "CommandLineParser.h"
#include "network/MPIChannel.h"
#include "network/ReceiveBuffer.h"
#include "serialization/utils.h"

#include <chrono>
#include <iostream>
#include <string>

#define MEGABYTE 1000000
#define RANK0 0

// Example ways to run this program:
// mpirun -n 6 -H localhost ./tideBenchmarkMPI --datasize 60 --packets 100
//
// Object size [Mbytes]: 60
// Time to send 100 objects: 3.445
// Time per object: 0.03445
// Throughput [Mbytes/sec]: 1741.66

namespace
{
class Timer
{
public:
    using clock = std::chrono::high_resolution_clock;

    void start() { _startTime = clock::now(); }
    float elapsed() const
    {
        const auto now = clock::now();
        return std::chrono::duration<float>{now - _startTime}.count();
    }

private:
    clock::time_point _startTime;
};

namespace po = boost::program_options;

class BenchmarkOptions : public CommandLineParser
{
public:
    BenchmarkOptions()
    {
        // clang-format off
        desc.add_options()
            ("datasize,s", po::value<float>()->default_value( 0.f ),
             "Size of each data packet [MB]")
            ("packets,p", po::value<size_t>()->default_value( 0u ),
             "number of packets to transmit")
        ;
        // clang-format on
    }
    size_t dataSize() const { return vm["datasize"].as<float>() * MEGABYTE; }
    size_t packetsCount() const { return vm["packets"].as<size_t>(); }
};
}

/**
 * Send data via MPI to benchmark the effective link speed at application level.
 */
int main(int argc, char** argv)
{
    COMMAND_LINE_PARSER_CHECK(BenchmarkOptions, "tideBenchmarkMPI");

    MPIChannel mpiChannel(argc, argv);

    // Send buffer
    std::vector<char> noiseBuffer(commandLine.dataSize());
    for (auto& elem : noiseBuffer)
        elem = rand();
    const auto serializedData = serialization::toBinary(noiseBuffer);

    // Receive buffer
    ReceiveBuffer buffer;
    Timer timer;
    size_t counter = 0;

    mpiChannel.globalBarrier();
    timer.start();

    while (counter < commandLine.packetsCount())
    {
        if (mpiChannel.getRank() == RANK0)
            mpiChannel.broadcast(MPIMessageType::NONE, serializedData);
        else
        {
            const MPIHeader header = mpiChannel.receiveHeader(RANK0);
            buffer.setSize(header.size);
            mpiChannel.receiveBroadcast(buffer.data(), header.size, RANK0);
        }
        ++counter;
    }

    const float time = timer.elapsed();

    if (mpiChannel.getRank() == RANK0)
    {
        std::cout << "Object size [Mbytes]: "
                  << (float)serializedData.size() / MEGABYTE << std::endl;
        std::cout << "Time to send " << counter << " objects: " << time
                  << std::endl;
        std::cout << "Time per object: " << time / counter << std::endl;
        std::cout << "Throughput [Mbytes/sec]: "
                  << counter * serializedData.size() / time / MEGABYTE
                  << std::endl;
    }

    return EXIT_SUCCESS;
}
