#!/bin/bash

# Auxiliary script for Travis CI building sView for Android platform

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}; if [ -d "${aScriptPath}" ]; then cd "$aScriptPath"; fi; aScriptPath="$PWD";

aParties=$aScriptPath/../3rdparty/android

if [ ! -x "$aParties/unpack.sh" ]; then
  git clone --depth 1 https://github.com/gkv311/sview-deps-android.git $aParties
fi;

# unpack archives and define variables
source $aParties/unpack.sh

if [ ! -d "${SVIEW_NDK}" ]; then exit 1; fi;

# define number of jobs from available CPU cores
aNbJobs="$(getconf _NPROCESSORS_ONLN)"

# perform building itself
make --directory=$aScriptPath/.. clean
make --directory=$aScriptPath/.. genjavares ANDROID_NDK=$SVIEW_NDK ANDROID_BUILD_TOOLS=$SVIEW_BUILD_TOOLS ANDROID_PLATFORM=$SVIEW_PLATFORM \
  FFMPEG_ROOT=$SVIEW_FFMPEG FREETYPE_ROOT=$SVIEW_FREETYPE OPENAL_ROOT=$SVIEW_OPENAL LIBCONFIG_ROOT=$SVIEW_LIBCONFIG WERROR_LEVEL=1
make --directory=$aScriptPath/.. sView_keystore_debug ANDROID_NDK=$SVIEW_NDK ANDROID_BUILD_TOOLS=$SVIEW_BUILD_TOOLS ANDROID_PLATFORM=$SVIEW_PLATFORM \
  FFMPEG_ROOT=$SVIEW_FFMPEG FREETYPE_ROOT=$SVIEW_FREETYPE OPENAL_ROOT=$SVIEW_OPENAL LIBCONFIG_ROOT=$SVIEW_LIBCONFIG WERROR_LEVEL=1
make --directory=$aScriptPath/.. -j $aNbJobs android ANDROID_NDK=$SVIEW_NDK ANDROID_BUILD_TOOLS=$SVIEW_BUILD_TOOLS ANDROID_PLATFORM=$SVIEW_PLATFORM \
  FFMPEG_ROOT=$SVIEW_FFMPEG FREETYPE_ROOT=$SVIEW_FREETYPE OPENAL_ROOT=$SVIEW_OPENAL LIBCONFIG_ROOT=$SVIEW_LIBCONFIG WERROR_LEVEL=1
