#!/bin/bash

# Small help to build libopenjpeg for MinGW [for sView project]
# https://www.openjpeg.org/

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}; if [ -d "${aScriptPath}" ]; then cd "$aScriptPath"; fi; aScriptPath="$PWD";
aSrcRoot=$aScriptPath

buildProject () {
  aPrefix=$1
  aBuildRoot="$aSrcRoot/build"
  aBuildPath="$aBuildRoot/openjpeg-${aPrefix}-make"
  aDistPath="$aBuildRoot/openjpeg-${aPrefix}"
  rm -f -r "$aBuildPath"
  rm -f -r "$aDistPath"
  rm -f -r "$aBuildRoot/openjpeg-${aPrefix}.7z"
  cmake -G "Ninja" \
   -D CMAKE_TOOLCHAIN_FILE="~/develop/${aPrefix}.cmake" \
   -D CMAKE_BUILD_TYPE:STRING="Release" \
   -D CMAKE_INSTALL_PREFIX:PATH="$aDistPath" \
   -D BUILD_SHARED_LIBS:BOOL="OFF" \
   -B "$aBuildPath" -S "$aSrcRoot"

  cmake --build "$aBuildPath" --config Release --target clean
  cmake --build "$aBuildPath" --config Release
  cmake --build "$aBuildPath" --config Release --target install
  
  cp -f "$aSrcRoot/LICENSE"   "$aDistPath"
  cp -f "$aSrcRoot/README.md" "$aDistPath"
  7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on "$aBuildRoot/openjpeg-${aPrefix}.7z" "$aDistPath"
}

set -o pipefail
buildProject "i686-w64-mingw32"
buildProject "x86_64-w64-mingw32"
