#!/bin/bash

# Just shortcut to generate Alpha version of sView
# (with time-bomb).
#
# Copyright © Kirill Gavrilov, 2010-2011

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}; if [ -d "${aScriptPath}" ]; then cd "$aScriptPath"; fi; aScriptPath="$PWD";

./build.sh ALPHA

aSystem=`uname -s`
if [ "$aSystem" != "Darwin" ]; then
    ./buildDeb.sh ALPHA $*
else
    ./buildMac.sh ALPHA $*
fi
