#!/bin/bash

# Small help to build dav1d for MinGW [for sView project]
# https://code.videolan.org/videolan/dav1d

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}; if [ -d "${aScriptPath}" ]; then cd "$aScriptPath"; fi; aScriptPath="$PWD";
aSrcRoot=$aScriptPath

buildProject () {
  aPrefix=$1
  aBuildRoot="$aSrcRoot/build"
  aBuildPath="$aBuildRoot/dav1d-${aPrefix}-make"
  aDistPath="$aBuildRoot/dav1d-${aPrefix}"
  rm -f -r "$aBuildPath"
  rm -f -r "$aDistPath"
  rm -f -r "$aBuildRoot/dav1d-${aPrefix}.7z"

  meson setup "${aBuildPath}" --cross-file=package/crossfiles/${aPrefix}.meson --default-library=static --prefix=$aDistPath
  pushd "${aBuildPath}"
  ninja
  ninja install
  popd
}

set -o pipefail
buildProject "i686-w64-mingw32"
buildProject "x86_64-w64-mingw32"
