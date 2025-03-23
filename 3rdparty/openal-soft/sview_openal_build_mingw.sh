#!/bin/bash

# go to the script directory
aSrcRoot=${BASH_SOURCE%/*}
if [ -d "$aSrcRoot" ]; then cd "$aSrcRoot"; fi
aSrcRoot="$PWD"

buildProject() {
  aPrefix=$1
  aBuildRoot="$aSrcRoot/build"
  aBuildPath="$aBuildRoot/openal-soft-${aPrefix}-make"
  aDistPath="$aBuildRoot/openal-soft-${aPrefix}"
  aLogFile="$aBuildRoot/openal-soft-${aPrefix}-build.log"
  rm -f -r "$aBuildPath"
  rm -f -r "$aDistPath"
  rm -f -r "$aBuildRoot/openal-soft-${aPrefix}.7z"
  rm    -f "$aLogFile"
  mkdir -p "$aBuildPath"

  set -o pipefail

  cmake -G "Ninja" \
 -D CMAKE_TOOLCHAIN_FILE:FILEPATH="~/develop/${aPrefix}.cmake" \
 -D CMAKE_BUILD_TYPE:STRING="Release" \
 -D CMAKE_INSTALL_PREFIX:PATH="$aDistPath" \
 -D ALSOFT_EXAMPLES:BOOL="OFF" \
 -D ALSOFT_TESTS:BOOL="OFF" \
 -D ALSOFT_UTILS:BOOL="OFF" \
 -D ALSOFT_BUILD_ROUTER:BOOL="OFF" \
 -D ALSOFT_REQUIRE_WINMM:BOOL="ON" \
 -D ALSOFT_REQUIRE_DSOUND:BOOL="ON" \
 -D ALSOFT_REQUIRE_WASAPI:BOOL="ON" \
 -D ALSOFT_STATIC_LIBGCC:BOOL="ON" \
 -D ALSOFT_STATIC_STDCXX:BOOL="ON" \
 -D ALSOFT_STATIC_WINPTHREAD:BOOL="ON" \
 -B "$aBuildPath" -S "$aSrcRoot" | tee -a "$aLogFile"

  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
  cmake --build "$aBuildPath" --config Release --target clean
  
  cmake --build "$aBuildPath" --config Release | tee -a "$aLogFile"
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi

  cmake --build "$aBuildPath" --config Release --target install | tee -a "$aLogFile"
  
  cp -f "$aSrcRoot/COPYING"   "$aDistPath"
  cp -f "$aSrcRoot/README.md" "$aDistPath"
  7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on "$aBuildRoot/openal-soft-${aPrefix}.7z" "$aDistPath"
}

#buildProject "i686-w64-mingw32-posix"
buildProject "x86_64-w64-mingw32-posix"
