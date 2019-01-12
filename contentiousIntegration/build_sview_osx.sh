#!/bin/bash

# Auxiliary script for Travis CI building sView for OS X platform

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}; if [ -d "${aScriptPath}" ]; then cd "$aScriptPath"; fi; aScriptPath="$PWD";

# define number of jobs from available CPU cores
aNbJobs="$(getconf _NPROCESSORS_ONLN)"

# perform building itself
make --directory=$aScriptPath/.. clean
make --directory=$aScriptPath/.. -j $aNbJobs WERROR_LEVEL=1
