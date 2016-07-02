#!/bin/bash

# small help to build libconfig for Android
# http://www.hyperrealm.com/libconfig/

# place up-to-date config.sub to libconfig-1.4.9/aux-build
# http://git.savannah.gnu.org/gitweb/?p=config.git;a=tree
#wget http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD
#wget http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD

#$HOME/develop/android-ndk-r12b/build/tools/make-standalone-toolchain.sh --platform=android-15 --install-dir=$HOME/develop/android15-armv7a --ndk-dir=$HOME/develop/android-ndk-r12b --toolchain=arm-linux-androideabi-4.9
#export PATH=$HOME/develop/android15-armv7a/bin:$PATH

export CC=arm-linux-androideabi-gcc
export CXX=arm-linux-androideabi-g++
export "CFLAGS=-march=armv7-a -mfloat-abi=softfp"
export "CXXFLAGS=-march=armv7-a -mfloat-abi=softfp"
./configure --host arm-linux-androideabi
