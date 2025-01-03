#!/bin/bash

# Auxiliary script for Travis CI building sView for Linux platform

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

cmake -G "Ninja Multi-Config" \
      -D BUILD_TREAT_WARNINGS_AS_ERRORS=ON \
      -D CMAKE_INSTALL_PREFIX="$aBuildPath/install" \
      -S "$aSrcRoot" -B "$aBuildPath"

cmake --build "$aBuildPath" --config Release --target clean
cmake --build "$aBuildPath" --config Release

cmake --build "$aBuildPath" --config Release --target install
