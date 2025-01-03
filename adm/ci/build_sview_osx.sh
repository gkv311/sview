#!/bin/bash

# Auxiliary script for Travis CI building sView for OS X platform

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}; if [ -d "${aScriptPath}" ]; then cd "$aScriptPath"; fi; aScriptPath="$PWD";

aSrcRoot=$aScriptPath/../..
aBuildPath=$aSrcRoot/build

# define number of jobs from available CPU cores
aNbJobs="$(getconf _NPROCESSORS_ONLN)"

# avoid implicit Android target build when NDK is installed system-wide
unset ANDROID_NDK

# perform building itself
#make --directory=$aSrcRoot clean
#make --directory=$aSrcRoot -j $aNbJobs WERROR_LEVEL=1

# libraries are expected to be found at /usr/local/
aBrewPrefix="$(brew --prefix)"
FREETYPE_DIR=$aBrewPrefix
FFMPEG_DIR=$aBrewPrefix
OPENAL_DIR=$aBrewPrefix/opt/openal-soft

#export CXXFLAGS=-Wno-error=cpp -Wno-error=deprecated-declarations
cmake -G "Unix Makefiles" \
      -D BUILD_TREAT_WARNINGS_AS_ERRORS=ON \
      -D CMAKE_BUILD_TYPE="Release" \
      -D CMAKE_INSTALL_PREFIX="$aBuildPath/install" \
      -D FREETYPE_DIR="$FREETYPE_DIR" \
      -D FFMPEG_DIR="$FFMPEG_DIR" \
      -D OPENAL_DIR="$OPENAL_DIR" \
      -D USE_OPENVR=OFF \
      -S "$aSrcRoot" -B "$aBuildPath"

cmake --build "$aBuildPath" --config Release --target clean
cmake --build "$aBuildPath" --config Release -- -j $aNbJobs

#cmake --build "$aBuildPath" --config Release --target install
