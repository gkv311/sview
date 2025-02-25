#!/bin/bash

# Small help to build libconfig for Android [for sView project].
# http://www.hyperrealm.com/libconfig/
# https://github.com/hyperrealm/libconfig

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}; if [ -d "${aScriptPath}" ]; then cd "$aScriptPath"; fi; aScriptPath="$PWD";

aProjName=libconfig-1.7.3

PATH=~/develop/tools/cmake-3.31.5-linux-x86_64/bin:$PATH

#CMAKE_ANDROID_NDK=~/develop/tools/android-ndk-r12b
CMAKE_ANDROID_NDK=~/develop/tools/android-ndk-r27c
CMAKE_BUILD_TYPE=Release

aSrcRoot=${aScriptPath}/${aProjName}.git
aBuildRoot=${aScriptPath}/android-make
aDestRoot=${aScriptPath}/android
aDestMulti=${aDestRoot}/${aProjName}
aCppLib=c++_shared
CMAKE_C_FLAGS=-fPIC

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
   -D CMAKE_C_FLAGS:STRING="$CMAKE_C_FLAGS" \
   -D CMAKE_CXX_FLAGS:STRING="$CMAKE_C_FLAGS" \
   -D BUILD_SHARED_LIBS:BOOL=OFF \
   -D BUILD_EXAMPLES:BOOL=OFF \
   -D BUILD_TESTS:BOOL=OFF \
   -B "$aBuildPath" -S "$aSrcRoot"
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi

  cmake --build "$aBuildPath" --config Release --target clean
  cmake --build "$aBuildPath" --config Release
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
  cmake --build "$aBuildPath" --config Release --target install

  cp -f "$aSrcRoot/LICENSE" "$CMAKE_INSTALL_PREFIX/"
  cp -f "$aSrcRoot/README"  "$CMAKE_INSTALL_PREFIX/"

  mkdir -p "$aDestMulti/libs/$anAbi"
  cp -f    "$CMAKE_INSTALL_PREFIX/lib/libconfig.a"   "$aDestMulti/libs/$anAbi/"
  cp -f    "$CMAKE_INSTALL_PREFIX/lib/libconfig++.a" "$aDestMulti/libs/$anAbi/"
  cp -f -r "$CMAKE_INSTALL_PREFIX/include" "$aDestMulti"
}

for anArchIter in armeabi-v7a arm64-v8a x86 x86_64; do buildArch $anArchIter 21; done
#buildArch armeabi-v7a 16; buildArch arm64-v8a 21; buildArch x86 16; buildArch x86_64 21

cp -f "$aSrcRoot/LICENSE" "$aDestMulti"
cp -f "$aSrcRoot/README"  "$aDestMulti"

#rm $aDestMulti/../${aProjName}-android.7z &>/dev/null
#7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on $aDestMulti/../${aProjName}-android.7z $aDestMulti
