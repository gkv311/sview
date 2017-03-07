#!/bin/bash

# go to the script directory
anOpenAlSrc=${BASH_SOURCE%/*}
if [ -d "$anOpenAlSrc" ]; then cd "$anOpenAlSrc"; fi
anOpenAlSrc="$PWD"

aNbJobs="$(getconf _NPROCESSORS_ONLN)"

anNdkPath=$HOME/develop/android-ndk-r12b
aToolchain=$HOME/develop/android-cmake/android.toolchain.cmake

cmakeBuildOpenAl() {
  anApi="$1"
  anAbi="$2"
  aPlatformAndCompiler=android-$anAbi
  aWorkDir=build/${aPlatformAndCompiler}-make
  aLogFile=$anOpenAlSrc/build-${aPlatformAndCompiler}.log

  mkdir -p $aWorkDir
  rm    -f $aLogFile

  pushd $aWorkDir

  set -o pipefail

  echo Configuring OpenAL-soft for Android...
  cmake -G "Unix Makefiles" \
 -D CMAKE_TOOLCHAIN_FILE:FILEPATH="$aToolchain" \
 -D ANDROID_NDK:FILEPATH="$anNdkPath" \
 -D ANDROID_ABI:STRING="$anAbi" \
 -D ANDROID_NATIVE_API_LEVEL:STRING="$anApi" \
 -D ANDROID_STL:STRING="gnustl_shared" \
 -D CMAKE_BUILD_TYPE:STRING="Release" \
 -D BUILD_LIBRARY_TYPE:STRING="Shared" \
 -D ALSOFT_BACKEND_OPENSL:BOOL="ON" \
 -D ALSOFT_REQUIRE_OPENSL:BOOL="ON" \
 -D ALSOFT_BACKEND_WAVE:BOOL="OFF" \
 -D ALSOFT_EXAMPLES:BOOL="OFF" \
 -D ALSOFT_TESTS:BOOL="OFF" \
 -D ALSOFT_UTILS:BOOL="OFF" \
 -D ALSOFT_NO_CONFIG_UTIL:BOOL="ON" \
 -D CMAKE_INSTALL_PREFIX:PATH="$anOpenAlSrc/build/$aPlatformAndCompiler" \
 "$anOpenAlSrc" | tee -a $aLogFile

  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
  make clean

  echo Building OpenAL-soft...
  make -j$aNbJobs | tee -a $aLogFile
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi

  echo Installing OpenAL into $anOpenAlSrc/work/$aPlatformAndCompiler...
  make install | tee -a $aLogFile

  popd
}

cmakeBuildOpenAl "15" "armeabi-v7a"
cmakeBuildOpenAl "15" "x86"
cmakeBuildOpenAl "21" "arm64-v8a"

OUTPUT_FOLDER="$anOpenAlSrc/install/openal-soft-android"
rm -f -r "$OUTPUT_FOLDER"
mkdir -p "$OUTPUT_FOLDER/include"
mkdir -p "$OUTPUT_FOLDER/libs/armeabi-v7a"
mkdir -p "$OUTPUT_FOLDER/libs/x86"
mkdir -p "$OUTPUT_FOLDER/libs/arm64-v8a"
cp -f    "$anOpenAlSrc/COPYING"           "$OUTPUT_FOLDER"
cp -f    "$anOpenAlSrc/README"            "$OUTPUT_FOLDER"
cp -f -r "$anOpenAlSrc/build/android-armeabi-v7a/include"          "$OUTPUT_FOLDER"
cp -f -r "$anOpenAlSrc/build/android-armeabi-v7a/share"            "$OUTPUT_FOLDER"
cp -f -L "$anOpenAlSrc/build/android-armeabi-v7a/lib/libopenal.so" "$OUTPUT_FOLDER/libs/armeabi-v7a"
cp -f -L "$anOpenAlSrc/build/android-x86/lib/libopenal.so"         "$OUTPUT_FOLDER/libs/x86"
cp -f -L "$anOpenAlSrc/build/android-arm64-v8a/lib/libopenal.so"   "$OUTPUT_FOLDER/libs/arm64-v8a"

rm $OUTPUT_FOLDER/../openal-soft-android.7z &>/dev/null
7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on $OUTPUT_FOLDER/../openal-soft-android.7z $OUTPUT_FOLDER
