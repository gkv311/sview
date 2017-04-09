#!/bin/bash

# small help to build zlib for MinGW [for sView project]
# http://zlib.net/

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

# 32-bit
OUTPUT_FOLDER="$aLibRoot/install/zlib-mingw-win32"
aPrefix=i686-w64-mingw32
rm -f -r "$OUTPUT_FOLDER"
mkdir -p "$OUTPUT_FOLDER/include"
mkdir -p "$OUTPUT_FOLDER/lib"
cp -f    "$aLibRoot/README" "$OUTPUT_FOLDER"
cp -f     $aLibRoot/*.h     "$OUTPUT_FOLDER/include"

echo "Output directory 32-bit: $OUTPUT_FOLDER"
export CC=${aPrefix}-gcc
export AR=${aPrefix}-ar
export RANLIB=${aPrefix}-ranlib
./configure --static
make clean
make -j$aNbJobs
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
cp -f "$aLibRoot/libz.a" "$OUTPUT_FOLDER/lib"

rm $OUTPUT_FOLDER/../zlib-mingw-win32.7z &>/dev/null
7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on $OUTPUT_FOLDER/../zlib-mingw-win32.7z $OUTPUT_FOLDER

# 64-bit
OUTPUT_FOLDER="$aLibRoot/install/zlib-mingw-win64"
aPrefix=x86_64-w64-mingw32
rm -f -r "$OUTPUT_FOLDER"
mkdir -p "$OUTPUT_FOLDER/include"
mkdir -p "$OUTPUT_FOLDER/lib"
cp -f    "$aLibRoot/README" "$OUTPUT_FOLDER"
cp -f     $aLibRoot/*.h     "$OUTPUT_FOLDER/include"

echo "Output directory 64-bit: $OUTPUT_FOLDER"
export CC=${aPrefix}-gcc
export AR=${aPrefix}-ar
export RANLIB=${aPrefix}-ranlib
./configure --static
make clean
make -j$aNbJobs
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
cp -f "$aLibRoot/libz.a" "$OUTPUT_FOLDER/lib"

rm $OUTPUT_FOLDER/../zlib-mingw-win64.7z &>/dev/null
7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on $OUTPUT_FOLDER/../zlib-mingw-win64.7z $OUTPUT_FOLDER

export "PATH=$aPathBak"
