#!/bin/bash
set -xe


# copy os specific spack configuration
cd $TRAVIS_BUILD_DIR
mkdir -p $HOME/.spack/
cp .travis/packages.$TRAVIS_OS_NAME.yaml $HOME/.spack/packages.yaml
cp .travis/modules.yaml $HOME/.spack/


# spack repository
cd $TRAVIS_BUILD_DIR
git clone --depth 1 https://github.com/pramodskumbhar/spack.git -b stable


# enables spack command line support
set +x
source $SPACK_ROOT/share/spack/setup-env.sh
set -x


# add spack packages repository
cd $TRAVIS_BUILD_DIR
git clone --depth 1 https://github.com/pramodskumbhar/spack-packages.git -b tide_proto
set +x
spack repo add --scope site ./spack-packages


# external packages are already installed on system this
# install step just register them to spack as external packages
spack install mpich openmpi boost zeromq qt python@2.7
