#!/bin/bash


# for travis_wait (long builds)
curdir="$(dirname "$0")"
source $curdir/travis_helper.sh


# for nightly/weekly builds we test multiple compilers and build options
if [ "$TRAVIS_PULL_REQUEST" == "false" ]; then

    if [ "$TRAVIS_OS_NAME" == "linux" ]; then
        packages=(
                "tide@develop -keyboard -touch -movie -rest -knl %gcc ^deflect@develop"
                "tide@develop +keyboard +touch +movie +rest -knl %gcc ^deflect@develop ^zeroeq@develop ^servus@develop"
        )
    else
        packages=(
                "tide@develop -keyboard -touch -movie -rest -knl %clang ^deflect@develop"
                "tide@develop +keyboard +touch +movie +rest -knl %clang ^deflect@develop ^zeroeq@develop ^servus@develop"
        )
    fi
# for pull request do minumum builds (i.e. single compiler per platform)
# this almost similar but one could change as needed
else

    if [ "$TRAVIS_OS_NAME" == "linux" ]; then
        packages=(
                "tide@develop -keyboard -touch -movie -rest -knl %gcc ^deflect@develop"
                "tide@develop +keyboard +touch +movie +rest -knl %gcc ^deflect@develop ^zeroeq@develop ^servus@develop"
        )
    else
        packages=(
                "tide@develop -keyboard -touch -movie -rest -knl %gcc ^deflect@develop"
                "tide@develop +keyboard +touch +movie +rest -knl %clang ^deflect@develop ^zeroeq@develop ^servus@develop"
        )
    fi

    # for pull request we want to build the PR branch/commit which is already
    # cloned. replace github url with local directory where Tide is cloned
    tide_package=$TRAVIS_BUILD_DIR/spack-packages/packages/tide/package.py
    sed -i'' -e "s#git=url#git='file:///$TRAVIS_BUILD_DIR'#g" $tide_package

fi


# initialize module
if [ "$TRAVIS_OS_NAME" == "linux" ]; then
    source /etc/profile.d/modules.sh
else
    source /usr/local/opt/modules/Modules/init/bash
fi


# to display build log in case of error
install_options="--show-log-on-error"


# for linux there are not installed
travis_wait spack install $install_options libjpeg-turbo
travis_wait spack install $install_options ffmpeg


# spack command line support
source $SPACK_ROOT/share/spack/setup-env.sh


# show tide package info
spack info tide


# number of packages being installed
echo " == > BUILDING ${#packages[*]} PACKAGES : "${packages[*]}""


# build all packages using spack
for package in "${packages[@]}"
do
    echo " == > PACKAGE SPEC : spack spec -I $package"
    spack spec -I $package

    # install package: travis has 4MB limit of stdout log and hence not using `-v`
    # other alternative is to use : (spack install -v $package >> build.log 2>&1; exit 0)
    # and then `cat build.lot` in case of failure
    echo " == > INSTALLING PACKAGE : spack install $package"
    travis_wait spack install $install_options $package

    # check if package installed properly
    if [[ `spack find $package` == *"No package matches"* ]];  then

        echo " == > PACKAGE INSTALLATION CHECK FAILED!"
        exit 1

    fi
done


# show all generated modules
echo " == > AUTOGENERATED MODULES : module avail"
module avail


if [ "$TRAVIS_OS_NAME" == "linux" ]; then
    name="tide/develop-gcc-mpich-movie-rest-touch-keyboard"
else
    name="tide/develop-clang-openmpi-movie-rest-touch-keyboard"
fi

echo " == > SAMPLE MODULE : module show $name"
module show $name
