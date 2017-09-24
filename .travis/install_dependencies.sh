#!/bin/bash

set -e
set -x

MPI_LIB_NAME="$1"

case "$TRAVIS_OS_NAME" in
    osx)
        brew install cppcheck doxygen
        brew install ffmpeg jpeg-turbo qt5
        brew install zeromq
        brew install modules

        case "$MPI_LIB_NAME" in
            mpich|mpich3)
                brew install mpich
                ;;
            openmpi)
                brew install openmpi
                ;;
            *)
                echo "ERROR: Unknown MPI Implementation: $MPI_LIB_NAME"
                exit 1
                ;;
        esac
    ;;

    linux)
        sudo apt-get update -q
        sudo apt-get install -y software-properties-common
        sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
        sudo add-apt-repository -y ppa:kvirc/kvirc-qt5.5
        sudo add-apt-repository -y ppa:kzemek/boost
        sudo apt-get update

        sudo apt-get install -y g++-5

        sudo apt-get install -y python imagemagick libboost1.58-dev \
             libboost-program-options1.58-dev libboost-serialization1.58-dev libboost-test1.58-dev \
             qtbase5-private-dev qtdeclarative5-dev libqt5serialport5-dev libqt5svg5-dev \
             libqt5webkit5-dev libqt5xmlpatterns5-dev libqt5x11extras5-dev qml-module-qtquick-controls \
             libpoppler-glib-dev libcairo2-dev libpoppler-qt5-dev librsvg2-dev libtiff5-dev \
             libavutil-dev libavformat-dev libavcodec-dev libswscale-dev qtdeclarative5-private-dev

        sudo apt-get install -y libboost1.58-dev libboost-atomic1.58-dev libboost-chrono1.58-dev \
              libboost-date-time1.58-dev libboost-filesystem1.58-dev libboost-regex1.58-dev \
              libboost-system1.58-dev libboost-thread1.58-dev libavutil-dev libav-tools

        sudo apt-get install -y nasm yasm libzmq3-dev

        case "$MPI_LIB_NAME" in
            mpich|mpich3)
                sudo apt-get install -y mpich libmpich-dev
                ;;
            openmpi)
                sudo apt-get install -y openmpi-bin libopenmpi-dev
                ;;
            *)
                echo "ERROR: Unknown MPI Implementation: $MPI_LIB_NAME"
                exit 1
                ;;
        esac
        ;;

    *)
        echo "Unknown Operating System: $os"
        exit 1
        ;;
esac
