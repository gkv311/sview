#!/bin/bash

# Auxiliary script for Travis CI building sView for Linux platform

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}; if [ -d "${aScriptPath}" ]; then cd "$aScriptPath"; fi; aScriptPath="$PWD";

aSrcRoot=$aScriptPath/../..
aBuildPath=$aSrcRoot/build

# avoid implicit Android target build when NDK is installed system-wide
unset ANDROID_NDK

cmake -G "Ninja Multi-Config" \
      -D BUILD_TREAT_WARNINGS_AS_ERRORS=ON \
      -D CMAKE_INSTALL_PREFIX="$aBuildPath/install" \
      -S "$aSrcRoot" -B "$aBuildPath"

cmake --build "$aBuildPath" --config Release --target clean
cmake --build "$aBuildPath" --config Release

cmake --build "$aBuildPath" --config Release --target install
