#!/bin/bash

# Small help to build dav1d for Android [for sView project].
# https://code.videolan.org/videolan/dav1d

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}; if [ -d "${aScriptPath}" ]; then cd "$aScriptPath"; fi; aScriptPath="$PWD";

aProjName=dav1d-1.5.1

aSrcRoot=${aScriptPath}/${aProjName}.git
aBuildRoot=${aScriptPath}/android-make
aDestRoot=${aScriptPath}/android
aDestMulti=${aDestRoot}/${aProjName}

#CMAKE_ANDROID_NDK=~/develop/tools/android-ndk-r12b
CMAKE_ANDROID_NDK=~/develop/tools/android-ndk-r27c
PATH=$CMAKE_ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH

rm -f -r "$aDestMulti"
mkdir -p "$aBuildRoot"

set -o pipefail

buildArch () {
  anAbi=$1

  aPrefix=
  if [ "$anAbi" == "arm64-v8a" ]; then
    aPrefix="aarch64-android"
  elif [ "$anAbi" == "armeabi-v7a" ]; then
    aPrefix="arm-android"
  else
    aPrefix="${anAbi}-android"
  fi

  aBuildPath=${aBuildRoot}/${aProjName}-${anAbi}-make
  CMAKE_INSTALL_PREFIX=${aDestRoot}/${aProjName}-${anAbi}
  rm -r -f ${aBuildPath}
  rm -r -f ${CMAKE_INSTALL_PREFIX}

  pushd "${aSrcRoot}"
  meson setup "${aBuildPath}" --cross-file=package/crossfiles/${aPrefix}.meson --default-library=static --prefix=$CMAKE_INSTALL_PREFIX
  popd
  pushd "${aBuildPath}"
  ninja
  ninja install
  popd

  cp -f "$aSrcRoot/COPYING"   "$CMAKE_INSTALL_PREFIX"
  cp -f "$aSrcRoot/README.md" "$CMAKE_INSTALL_PREFIX"

  mkdir -p "$aDestMulti/libs/$anAbi"
  cp -f    "$CMAKE_INSTALL_PREFIX/lib/libdav1d.a" "$aDestMulti/libs/$anAbi/"
  cp -f -r "$CMAKE_INSTALL_PREFIX/include" "$aDestMulti"
}

for anArchIter in armeabi-v7a arm64-v8a x86 x86_64; do buildArch $anArchIter 21; done

cp -f "$aSrcRoot/COPYING"   "$aDestMulti"
cp -f "$aSrcRoot/README.md" "$aDestMulti"
