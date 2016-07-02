#!/bin/bash

# go to the script directory
anOpenAlSrc=${BASH_SOURCE%/*}
if [ -d "$anOpenAlSrc" ]; then cd "$anOpenAlSrc"; fi
anOpenAlSrc="$PWD"

#$HOME/develop/android-ndk-r12b/build/tools/make-standalone-toolchain.sh --platform=android-15 --install-dir=$HOME/develop/android15-armv7a --ndk-dir=$HOME/develop/android-ndk-r12b --toolchain=arm-linux-androideabi-4.9

aToolchain=$HOME/develop/android-cmake/android.toolchain.cmake

anApi="15"
anAbi="armeabi-v7a"
aPlatformAndCompiler=android-$anAbi
aWorkDir=build/${aPlatformAndCompiler}-make
aLogFile=$anOpenAlSrc/build-${aPlatformAndCompiler}.log

aNbJobs=1

mkdir -p $aWorkDir
rm    -f $aLogFile

pushd $aWorkDir

set -o pipefail

echo Configuring OpenAL-soft for Android...
cmake -G "Unix Makefiles" \
 -D CMAKE_TOOLCHAIN_FILE:FILEPATH="$aToolchain" \
 -D ANDROID_STANDALONE_TOOLCHAIN:FILEPATH="$HOME/develop/android15-armv7a" \
 -D CMAKE_BUILD_TYPE:STRING="Release" \
 -D BUILD_LIBRARY_TYPE:STRING="Shared" \
 -D ALSOFT_BACKEND_OPENSL:BOOL="ON" \
 -D ALSOFT_REQUIRE_OPENSL:BOOL="ON" \
 -D ALSOFT_BACKEND_WAVE:BOOL="OFF" \
 -D ALSOFT_EXAMPLES:BOOL="OFF" \
 -D ALSOFT_TESTS:BOOL="OFF" \
 -D ALSOFT_UTILS:BOOL="OFF" \
 -D ALSOFT_NO_CONFIG_UTIL:BOOL="ON" \
 -D ANDROID_ABI:STRING="$anAbi" \
 -D ANDROID_NATIVE_API_LEVEL:STRING="$anApi" \
 -D ANDROID_STL:STRING="gnustl_shared" \
 -D CMAKE_INSTALL_PREFIX:PATH="$anOpenAlSrc/build/$aPlatformAndCompiler" \
 "$anOpenAlSrc" | tee -a $aLogFile

#-D ANDROID_NDK:FILEPATH="$HOME/develop/android-ndk-r12b"

aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi

# make clean build
make clean

echo Building OpenAL-soft...
make -j $aNbJobs | tee -a $aLogFile
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi

echo Installing OCCT into $anOpenAlSrc/work/$aPlatformAndCompiler...
make install | tee -a $aLogFile

#$HOME/develop/android-ndk-r12b/ndk-depends $anOpenAlSrc/build/$aPlatformAndCompiler/libopenal.so

popd
