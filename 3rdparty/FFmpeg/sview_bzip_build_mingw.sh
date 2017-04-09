#!/bin/bash

# small help to build bzip for MinGW [for sView project]
# http://www.bzip.org/

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}
if [ -d "$aScriptPath" ]; then
  cd "$aScriptPath"
fi

# define number of jobs from available CPU cores
aNbJobs="$(getconf _NPROCESSORS_ONLN)"
set -o pipefail

aPathBak="$PATH"
aLibRoot="$PWD"

# apply the patch
if grep -q 'sys\\stat.h' $aLibRoot/bzip2.c; then
  cp -f $aLibRoot/bzip2.c $aLibRoot/bzip2.c.bak
  sed -i 's/sys\\stat.h/sys\/stat.h/g' bzip2.c
fi

# 32-bit
OUTPUT_FOLDER="$aLibRoot/install/bzip-mingw-win32"
aPrefix=i686-w64-mingw32
rm -f -r "$OUTPUT_FOLDER"
mkdir -p "$OUTPUT_FOLDER/include"
mkdir -p "$OUTPUT_FOLDER/lib"
cp -f    "$aLibRoot/LICENSE" "$OUTPUT_FOLDER"
cp -f    "$aLibRoot/README"  "$OUTPUT_FOLDER"
cp -f    "$aLibRoot/bzlib.h" "$OUTPUT_FOLDER/include"

echo "Output directory 32-bit: $OUTPUT_FOLDER"
make clean
make libbz2.a -j$aNbJobs CC=${aPrefix}-gcc AR=${aPrefix}-ar RANLIB=${aPrefix}-ranlib
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
cp -f "$aLibRoot/libbz2.a" "$OUTPUT_FOLDER/lib"

rm $OUTPUT_FOLDER/../bzip-mingw-win32.7z &>/dev/null
7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on $OUTPUT_FOLDER/../bzip-mingw-win32.7z $OUTPUT_FOLDER

# 64-bit
OUTPUT_FOLDER="$aLibRoot/install/bzip-mingw-win64"
aPrefix=x86_64-w64-mingw32
rm -f -r "$OUTPUT_FOLDER"
mkdir -p "$OUTPUT_FOLDER/include"
mkdir -p "$OUTPUT_FOLDER/lib"
cp -f    "$aLibRoot/LICENSE" "$OUTPUT_FOLDER"
cp -f    "$aLibRoot/README"  "$OUTPUT_FOLDER"
cp -f    "$aLibRoot/bzlib.h" "$OUTPUT_FOLDER/include"

echo "Output directory 64-bit: $OUTPUT_FOLDER"
make clean
make libbz2.a -j$aNbJobs CC=${aPrefix}-gcc AR=${aPrefix}-ar RANLIB=${aPrefix}-ranlib
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
cp -f "$aLibRoot/libbz2.a" "$OUTPUT_FOLDER/lib"

rm $OUTPUT_FOLDER/../bzip-mingw-win64.7z &>/dev/null
7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on $OUTPUT_FOLDER/../bzip-mingw-win64.7z $OUTPUT_FOLDER

export "PATH=$aPathBak"
