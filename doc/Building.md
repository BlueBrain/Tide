Building Tide {#building}
============

Tide uses CMake 3.1 or later to create a platform-specific build environment.

## Building from Source

The standard build procedure using git and CMake is the following:

    git clone https://github.com/BlueBrain/Tide.git
    mkdir Tide/build
    cd Tide/build
    cmake ..
    make

## External Dependencies

Tide builds on top of standard and well tested libraries for core
functionalities such as rendering, serialization, file handling and network
communication. Those dependencies are:
* Boost 1.54 or later
* MPI with MPI_THREAD_MULTIPLE support (openmpi 1.6.5 or later recommended)
* Qt 5.4 or 5.5 (versions 5.6 and 5.7 currently have some
  [stability issues](https://github.com/BlueBrain/Tide/issues/80))

In addition, it also depends on some external projects that are automatically
cloned by CMake during the configure step. They come with their own additional
requirements:
* Deflect: streaming of contents and applications
  - libjpeg-turbo
* TUIO: multitouch interface
* VirtualKeyboard: on-screen virtual keyboard (Qml)
* ZeroEQ: http / REST interface (technically optional, but still needed to
  operate on-screen Launcher panel)
  - Pthreads
  - ZeroMQ

Some additional libraries are searched by CMake to enable useful extra
features if they are found:
* Cairo + RSVG (>= 2.36.2): optimal %SVG support (QtSVG used as a fallback)
* Cairo + poppler-glib: optimal PDF support
* poppler-qt5: fallback PDF support
* FFMPEG: playing movies
* TIFF: display large images (TIFF image pyramids)

### Automatic Package Installation

To ease the process of searching and installing all the necessary system
packages, an automatic installation process is provided. It is triggered by
passing a specific argument to CMake during the configuration step.
Currently, only Ubuntu (via apt-get) and OSX (via macports) are supported.

To run the automatic install, do the following:

    cd Tide/build
    cmake .. -DINSTALL_PACKAGES=1

One could also look directly at the TIDE_DEB_DEPENDS and TIDE_PORT_DEPENDS
entries in the top-level CMakeLists.txt of each (sub)project and install the
packages manually.

## Supported Platforms

This section gives more detailed information on building the software on some
popular platforms.

Unfortunately, many Linux distributions lack at least one of the required
components which must be installed manually. New users are advised to consider
Ubuntu 16.04 which is one of the simplest option overall.

### Ubuntu 16.04

Tide works almost out of the box on Ubuntu 16.04. The only issue is that the
openmpi / mpich packages are either buggy or lack support for
MPI_THREAD_MULTIPLE.

To boostrap the installation of Tide a on fresh install, do:

    sudo apt install git cmake
    git clone https://github.com/BlueBrain/Tide.git
    mkdir Tide/build
    cd Tide/build
    cmake .. -DINSTALL_PACKAGES=1

After this step all the necessary system packages should be installed. Then use
the following receipe to build and use a proper openmpi replacement:

    VERSION=1.10.3

    OPENMPI_MIRROR=http://www.open-mpi.org/software/ompi/v1.10/downloads
    OPENMPI_FOLDER=openmpi-${VERSION}
    OPENMPI_TARBALL=${OPENMPI_FOLDER}.tar.gz

    wget ${OPENMPI_MIRROR}/${OPENMPI_TARBALL}
    tar xzf ${OPENMPI_TARBALL}
    cd ${OPENMPI_FOLDER}

    ./configure --enable-mpi-thread-multiple --prefix=$HOME/opt/openmpi
    make install

And export the corresponding environment variables (add in ~/.bashrc):

    OPENMPI_ROOT=$HOME/opt/openmpi
    export PATH=$OPENMPI_ROOT/bin:$PATH
    export LD_LIBRARY_PATH=$OPENMPI_ROOT/lib:$LD_LIBRARY_PATH

Note: older openmpi versions have been tested to work as well (1.8.7, 1.6.5).

Check that the correct openmpi version is in the PATH:
> which mpiexec
and clear the CMakeCache (or just start with a fresh build folder) to ensure
that CMake does not use the incompatible system openmpi.

### Ubuntu 14.04

Tide has been extensively developed and tested on Ubuntu 14.04. Only a few
system packages are outdated and must be replaced:
* CMake 2.8: download recent CMake 3 binaries
  [from here](https://cmake.org/download/) and export PATH to *cmake* executable
* Qt 5.2.1: use Qt 5.4.1 (or 5.5.1) installer downloaded from
  [the Qt website](http://download.qt.io) and export (add in ~/.bashrc):

      QT_HOME=$HOME/Qt5.4.1/5.4/gcc_64
      export PATH=$QT_HOME/bin:$PATH
      export LD_LIBRARY_PATH=$QT_HOME/lib:$LD_LIBRARY_PATH
      export QML_IMPORT_PATH=$QT_HOME/qml

* cppcheck + lcov: build custom cppcheck-1.68 and lcov-1.11 (or newer) and
  export PATH to the binaries (only needed for *tests* and *doxygen* targets)

### RedHat 6

Tide has been built and used in production on RHEL 6.7 machines. However, the
deployment requires a large amount of Linux modules to replace the outdated
system packages and even then some of the latest features remain unavailable
(Cairo backend). This system is therefore not recommended for new deployments.

For those interested, here is a list of modules used at BlueBrain at the time of
writing (September 2016):
* cmake/3.2.3
* gcc/4.8.3 (+ gdb/7.5.1)
* boost/1.54.0
* openmpi/1.8.7
* qt/5.4.1
* poppler/0.45.0
* libzmq/4.0.6
* python/3.4.3-1
* doxygen/1.8.9.1
* cppcheck/1.68
* lcov/1.11

### MacOS

Tide builds and runs on OSX (x86_64), but has limited features and testing.

The required libjpeg-turbo cannot be installed via macports because it conflicts
with the regular libjpeg. A separate installer must be obtained from
[sourceforge](https://sourceforge.net/projects/libjpeg-turbo/).

### Windows

There is no plan to support Windows at the moment.

However, since most dependencies are open-source and cross-platform, one might
be able to build a Windows version of Tide given sufficient time and effort.
