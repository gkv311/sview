#!/bin/bash

# Small help to build mbedtls for Android platform [for sView project].
# https://tls.mbed.org/
# https://github.com/Mbed-TLS/mbedtls

# x86 build is broken on old gcc due to https://github.com/ARMmbed/mbedtls/issues/1910
# include/mbedtls/bn_mul.h
# -#if defined(__i386__) && defined(__OPTIMIZE__) && !defined(__ANDROID__)
# +#if defined(__i386__) && defined(__OPTIMIZE__) && !defined(__ANDROID__)
#
# #define MULADDC_INIT

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}; if [ -d "${aScriptPath}" ]; then cd "$aScriptPath"; fi; aScriptPath="$PWD";

aProjName=mbedtls-3.6.2

PATH=~/develop/tools/cmake-3.31.5-linux-x86_64/bin:$PATH

#CMAKE_ANDROID_NDK=~/develop/tools/android-ndk-r12b
CMAKE_ANDROID_NDK=~/develop/tools/android-ndk-r27c
CMAKE_BUILD_TYPE=Release

aSrcRoot=${aScriptPath}/${aProjName}.git
aBuildRoot=${aScriptPath}/android-make
aDestRoot=${aScriptPath}/android
aDestMulti=${aDestRoot}/${aProjName}
aCppLib=c++_shared

rm -f -r "$aDestMulti"
mkdir -p "$aBuildRoot"

set -o pipefail

function buildArch {
  anAbi=$1
  anApi=$2

  aBuildPath=${aBuildRoot}/${aProjName}-${anAbi}-make
  CMAKE_INSTALL_PREFIX=${aDestRoot}/${aProjName}-${anAbi}
  rm -r -f ${aBuildPath}
  rm -r -f ${CMAKE_INSTALL_PREFIX}

  cmake -G "Ninja" \
   -D CMAKE_SYSTEM_NAME:STRING="Android" \
   -D CMAKE_ANDROID_NDK="$CMAKE_ANDROID_NDK" \
   -D CMAKE_BUILD_TYPE:STRING="$CMAKE_BUILD_TYPE" \
   -D CMAKE_ANDROID_ARCH_ABI:STRING="$anAbi" \
   -D CMAKE_SYSTEM_VERSION:STRING="$anApi" \
   -D CMAKE_ANDROID_STL_TYPE="$aCppLib" \
   -D CMAKE_INSTALL_PREFIX:STRING="$CMAKE_INSTALL_PREFIX" \
   -D LIBRARY_OUTPUT_PATH:PATH="$aDestMulti/libs/$anAbi" \
   -D USE_SHARED_MBEDTLS_LIBRARY:BOOL="OFF" \
   -D USE_STATIC_MBEDTLS_LIBRARY:BOOL="ON" \
   -D ENABLE_TESTING:BOOL="OFF" \
   -D ENABLE_PROGRAMS:BOOL="OFF" \
   -B "$aBuildPath" -S "$aSrcRoot"
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi

  cmake --build "$aBuildPath" --config Release --target clean
  cmake --build "$aBuildPath" --config Release
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
  cmake --build "$aBuildPath" --config Release --target install

  cp -f "$aSrcRoot/LICENSE"   "$CMAKE_INSTALL_PREFIX/"
  cp -f "$aSrcRoot/README.md" "$CMAKE_INSTALL_PREFIX/"
  cp -f "$aSrcRoot/ChangeLog" "$CMAKE_INSTALL_PREFIX/"

  mkdir -p "$aDestMulti/libs/$anAbi"
  #cp -f    "$CMAKE_INSTALL_PREFIX/lib/libmbedtls.a" "$aDestMulti/libs/$anAbi/"
  cp -f -r "$CMAKE_INSTALL_PREFIX/include" "$aDestMulti"
}

for anArchIter in armeabi-v7a arm64-v8a x86 x86_64; do buildArch $anArchIter 21; done
#buildArch armeabi-v7a 16; buildArch arm64-v8a 21; buildArch x86 16; buildArch x86_64 21

cp -f "$aSrcRoot/LICENSE"   "$aDestMulti"
cp -f "$aSrcRoot/README.md" "$aDestMulti"
cp -f "$aSrcRoot/ChangeLog" "$aDestMulti"

#rm $aDestMulti/../${aProjName}-android.7z &>/dev/null
#7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on $aDestMulti/../${aProjName}-android.7z $aDestMulti
