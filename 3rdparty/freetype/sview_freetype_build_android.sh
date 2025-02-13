#!/bin/bash

# Small help to build FreeType for Android [for sView project].
# https://www.freetype.org

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}; if [ -d "${aScriptPath}" ]; then cd "$aScriptPath"; fi; aScriptPath="$PWD";

aProjName=freetype-2.10.4

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
   -D BUILD_SHARED_LIBS:BOOL=ON \
   -D FT_WITH_ZLIB=ON      -D CMAKE_DISABLE_FIND_PACKAGE_ZLIB:BOOL=OFF \
   -D FT_WITH_BZIP2=OFF    -D CMAKE_DISABLE_FIND_PACKAGE_BZip2:BOOL=TRUE \
   -D FT_WITH_PNG=OFF      -D CMAKE_DISABLE_FIND_PACKAGE_PNG:BOOL=TRUE \
   -D FT_WITH_HARFBUZZ=OFF -D CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz:BOOL=TRUE \
   -D FT_WITH_BROTLI=OFF   -D CMAKE_DISABLE_FIND_PACKAGE_BrotliDec:BOOL=TRUE \
   -B "$aBuildPath" -S "$aSrcRoot"
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi

  cmake --build "$aBuildPath" --config Release --target clean
  cmake --build "$aBuildPath" --config Release
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
  cmake --build "$aBuildPath" --config Release --target install

  cp -f "$aSrcRoot/docs/FTL.TXT"   "$CMAKE_INSTALL_PREFIX/"
  cp -f "$aSrcRoot/docs/GPLv2.TXT" "$CMAKE_INSTALL_PREFIX/"
  cp -f "$aSrcRoot/docs/CHANGES"   "$CMAKE_INSTALL_PREFIX/"
  cp -f "$aSrcRoot/docs/README"    "$CMAKE_INSTALL_PREFIX/"
  if [ -f "$aSrcRoot/LICENSE.TXT" ]; then
    cp -f "$aSrcRoot/LICENSE.TXT"  "$CMAKE_INSTALL_PREFIX/"
  else
    cp -f "$aSrcRoot/docs/LICENSE.TXT" "$CMAKE_INSTALL_PREFIX/"
  fi

  mkdir -p "$aDestMulti/libs/$anAbi"
  cp -f "$CMAKE_INSTALL_PREFIX/lib/libfreetype.so" "$aDestMulti/libs/$anAbi/"
  cp -f "$CMAKE_INSTALL_PREFIX/LICENSE.TXT"        "$aDestMulti/"
}

for anArchIter in armeabi-v7a arm64-v8a x86 x86_64; do buildArch $anArchIter 21; done
#buildArch armeabi-v7a 16; buildArch arm64-v8a 21; buildArch x86 16; buildArch x86_64 21

mkdir -p "$aDestMulti/include"
cp -f    "$aSrcRoot/README"         "$aDestMulti"
cp -f    "$aSrcRoot/docs/FTL.TXT"   "$aDestMulti"
cp -f    "$aSrcRoot/docs/GPLv2.TXT" "$aDestMulti"
cp -f    "$aSrcRoot/docs/CHANGES"   "$aDestMulti"
cp -f -r "$aSrcRoot/include"        "$aDestMulti"
rm -f -r "$aDestMulti/include/freetype/internal"

#rm $aDestMulti/../${aProjName}-android.7z &>/dev/null
#7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on $aDestMulti/../${aProjName}-android.7z $aDestMulti
