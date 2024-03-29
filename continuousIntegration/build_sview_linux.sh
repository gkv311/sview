#!/bin/bash

# Auxiliary script for Travis CI building sView for Linux platform

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}; if [ -d "${aScriptPath}" ]; then cd "$aScriptPath"; fi; aScriptPath="$PWD";

# define number of jobs from available CPU cores
aNbJobs="$(getconf _NPROCESSORS_ONLN)"

# avoid implicit Android target build when NDK is installed system-wide
unset ANDROID_NDK

# perform building itself
make --directory=$aScriptPath/.. clean
make --directory=$aScriptPath/.. -j $aNbJobs WERROR_LEVEL=1
