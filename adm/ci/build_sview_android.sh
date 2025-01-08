#!/bin/bash

# Auxiliary script for Travis CI building sView for Android platform

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}; if [ -d "${aScriptPath}" ]; then cd "$aScriptPath"; fi; aScriptPath="$PWD";

aSrcRoot=$aScriptPath/../..
aBuildPath=$aSrcRoot/build

aParties=$aSrcRoot/3rdparty/android

if [ ! -x "$aParties/unpack.sh" ]; then
  git clone --depth 1 https://github.com/gkv311/sview-deps-android.git $aParties
fi;

# unpack archives and define variables
source $aParties/unpack.sh

if [ ! -d "${SVIEW_NDK}" ]; then exit 1; fi;

# define number of jobs from available CPU cores
aNbJobs="$(getconf _NPROCESSORS_ONLN)"

# armeabi-v7a arm64-v8a x86 x86_64
#SVIEW_ANDROID_API=21
#SVIEW_ANDROID_ARCH=arm64-v8a
SVIEW_ANDROID_API=15
SVIEW_ANDROID_ARCH=armeabi-v7a
aCompiler=gcc
aCppLib=gnustl_shared
if [[ ! -d "${SVIEW_NDK}/sources/cxx-stl/gnu-libstdc++" ]]; then
  if [[ -d "${SVIEW_NDK}/sources/cxx-stl/llvm-libc++" ]]; then
    aCompiler=clang
    aCppLib=c++_shared
  fi
fi

# perform building itself
#make --directory=$aSrcRoot clean
#make --directory=$aSrcRoot genjavares ANDROID_NDK=$SVIEW_NDK ANDROID_BUILD_TOOLS=$SVIEW_BUILD_TOOLS ANDROID_PLATFORM=$SVIEW_PLATFORM \
#  FFMPEG_ROOT=$SVIEW_FFMPEG FREETYPE_ROOT=$SVIEW_FREETYPE OPENAL_ROOT=$SVIEW_OPENAL LIBCONFIG_ROOT=$SVIEW_LIBCONFIG WERROR_LEVEL=1
#make --directory=$aSrcRoot sView_keystore_debug ANDROID_NDK=$SVIEW_NDK ANDROID_BUILD_TOOLS=$SVIEW_BUILD_TOOLS ANDROID_PLATFORM=$SVIEW_PLATFORM \
#  FFMPEG_ROOT=$SVIEW_FFMPEG FREETYPE_ROOT=$SVIEW_FREETYPE OPENAL_ROOT=$SVIEW_OPENAL LIBCONFIG_ROOT=$SVIEW_LIBCONFIG WERROR_LEVEL=1
#make --directory=$aSrcRoot -j $aNbJobs android ANDROID_NDK=$SVIEW_NDK ANDROID_BUILD_TOOLS=$SVIEW_BUILD_TOOLS ANDROID_PLATFORM=$SVIEW_PLATFORM \
#  FFMPEG_ROOT=$SVIEW_FFMPEG FREETYPE_ROOT=$SVIEW_FREETYPE OPENAL_ROOT=$SVIEW_OPENAL LIBCONFIG_ROOT=$SVIEW_LIBCONFIG WERROR_LEVEL=1

cmake -G "Ninja Multi-Config" \
      -D BUILD_TREAT_WARNINGS_AS_ERRORS=ON \
      -D CMAKE_BUILD_TYPE="Release" \
      -D CMAKE_INSTALL_PREFIX="$aBuildPath/install" \
      -D CMAKE_SYSTEM_NAME:STRING="Android" \
      -D CMAKE_ANDROID_NDK="${SVIEW_NDK}" \
      -D CMAKE_BUILD_TYPE:STRING="Release" \
      -D CMAKE_ANDROID_ARCH_ABI:STRING="${SVIEW_ANDROID_ARCH}" \
      -D CMAKE_SYSTEM_VERSION:STRING="${SVIEW_ANDROID_API}" \
      -D CMAKE_ANDROID_STL_TYPE="$aCppLib" \
      -D ANDROID_PLATFORM:PATH="${SVIEW_PLATFORM}" \
      -D ANDROID_BUILD_TOOLS:PATH="${SVIEW_BUILD_TOOLS}" \
      -D FREETYPE_DIR="$SVIEW_FREETYPE" \
      -D FFMPEG_DIR="$SVIEW_FFMPEG" \
      -D OPENAL_DIR="$SVIEW_OPENAL" \
      -D LIBCONFIGCPP_DIR="$SVIEW_LIBCONFIG" \
      -D USE_OPENVR=OFF \
      -S "$aSrcRoot" -B "$aBuildPath"

cmake --build "$aBuildPath" --config Release --target clean
cmake --build "$aBuildPath" --config Release --target sView_keystore_debug
cmake --build "$aBuildPath" --config Release
