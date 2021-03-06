Building Tide {#building}
============

Tide uses CMake 3.1 or later to create a platform-specific build environment.

## Building from Source

The standard build procedure using git and CMake is the following:

    git clone --recursive https://github.com/BlueBrain/Tide.git
    mkdir Tide/build
    cd Tide/build
    cmake -GNinja -DCLONE_SUBPROJECTS=ON ..
    ninja

## External Dependencies

Tide builds on top of standard and well tested libraries for core
functionalities such as rendering, serialization, file handling and network
communication. These dependencies are:
* Boost 1.56 or later
* MPI with MPI_THREAD_MULTIPLE support (OpenMPI >= 1.6.5, or IntelMPI for
  InfiniBand networks)
* Qt 5.4 or later (5.9 or later recommended)

In addition, it also depends on some external projects that are automatically
cloned by CMake during the configure step. They come with their own additional
requirements:
* Deflect: streaming of contents and applications
  - libjpeg-turbo
* Rockets: http / REST interface (technically optional, but needed to operate
  the on-screen Launcher panel)
  - libwebsockets
* VirtualKeyboard: on-screen virtual keyboard (Qml)

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
    cmake .. -DINSTALL_PACKAGES=ON

Alternatively, look directly at the TIDE_DEB_DEPENDS and TIDE_PORT_DEPENDS
entries in the top-level CMakeLists.txt of each (sub)project and install the
packages manually.

## Supported Platforms

This section gives more detailed information on building the software on popular
platforms.

The current (Dec. 2018) reference platform at BBP is Ubuntu 16.04 with a custom
install of [Qt 5.9.7](https://download.qt.io/archive/qt/5.9/5.9.7/) and openmpi
(see below).

### Ubuntu 18.04

Tide compiles out of the box on Ubuntu 18.04 as all the dependencies are
available as system packages but it hasn't been tested extensively. There are
also some troubles with the system Qt (see below).

To compile Tide from scratch on a fresh system, installing all its dependencies,
do:

    sudo apt install git cmake build-essential
    git clone --recursive https://github.com/BlueBrain/Tide.git
    mkdir Tide/build
    cd Tide/build
    cmake .. -DCLONE_SUBPROJECTS=ON -DINSTALL_PACKAGES=ON
    make -j8

Known issue: at the time of writing (Dec. 2018) the webbrowser is crashing when
using the system Qt packages. Using a custom install of Qt downloaded from the
official website solves that problem.

### Ubuntu 16.04

Tide works almost out of the box on Ubuntu 16.04. The main issue is that the
openmpi and mpich packages are either buggy or lack support for
MPI_THREAD_MULTIPLE. A more recent Qt version is also recommeneded.

To boostrap the installation of Tide a on fresh install, do:

    sudo apt install git cmake build-essential
    git clone --recursive https://github.com/BlueBrain/Tide.git
    mkdir Tide/build
    cd Tide/build
    cmake .. -DCLONE_SUBPROJECTS=ON -DINSTALL_PACKAGES=ON

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

Warning: It seems that OpenMPI does not build correctly if OpenCL is installed.
The only know solution is to unistall OpenCL, build openmpi then re-install
OpenCL.

Alternative: If the above does not work for you, try installing the libmpich-dev
system package. Tide will then only run in single-window mode (using
configuration_1x1.json).

### MacOS

Tide builds and runs on OSX (x86_64), but has limited features and testing.

Most dependencies are easily installed through either brew or macports.

#### Brew

Using brew on OSX 10.13 / 10.14:

    brew install cmake ninja pkgconfig
    brew install boost ffmpeg librsvg open-mpi poppler qt jpeg-turbo libwebsockets
    export PATH=/usr/local/opt/qt/bin:$PATH
    export PKG_CONFIG_PATH=/usr/local/opt/jpeg-turbo/lib/pkgconfig
    export CPATH=$(brew --prefix openssl)/include

#### MacPorts

The required libjpeg-turbo cannot be installed via macports because it conflicts
with the regular libjpeg. A separate installer can be obtained from
[sourceforge](https://sourceforge.net/projects/libjpeg-turbo/).

### Windows

There is no plan to support Windows at the moment.

However, since most dependencies are open-source and cross-platform, one might
be able to build a Windows version of Tide given sufficient time and effort.
