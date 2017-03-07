#!/bin/bash

# small help to build libconfig for Android [for sView project]
# http://www.hyperrealm.com/libconfig/

# place up-to-date config.sub to libconfig-1.4.9/aux-build
# http://git.savannah.gnu.org/gitweb/?p=config.git;a=tree
#wget http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD
#wget http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD

#$HOME/develop/android-ndk-r12b/build/tools/make-standalone-toolchain.sh --platform=android-15 --install-dir=$HOME/develop/android15-armv7a  --ndk-dir=$HOME/develop/android-ndk-r12b --toolchain=arm-linux-androideabi-4.9
#$HOME/develop/android-ndk-r12b/build/tools/make-standalone-toolchain.sh --platform=android-21 --install-dir=$HOME/develop/android21-aarch64 --ndk-dir=$HOME/develop/android-ndk-r12b --toolchain=aarch64-linux-android-4.9
#$HOME/develop/android-ndk-r12b/build/tools/make-standalone-toolchain.sh --platform=android-15 --install-dir=$HOME/develop/android15-x86     --ndk-dir=$HOME/develop/android-ndk-r12b --toolchain=x86-4.9

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
aCFlagsArmv7a="-O2 -march=armv7-a -mfloat-abi=softfp"
aCFlagsArmv8a="-O2 -march=armv8-a"
aCFlagsx86="-O2 -march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32"

OUTPUT_FOLDER="$aLibRoot/install/libconfig-android"
rm -f -r "$OUTPUT_FOLDER"
mkdir -p "$OUTPUT_FOLDER/include"
mkdir -p "$OUTPUT_FOLDER/libs/armeabi-v7a"
mkdir -p "$OUTPUT_FOLDER/libs/x86"
mkdir -p "$OUTPUT_FOLDER/libs/arm64-v8a"
cp -f    "$aLibRoot/COPYING.LIB"       "$OUTPUT_FOLDER"
cp -f    "$aLibRoot/README"            "$OUTPUT_FOLDER"
cp -f    "$aLibRoot/lib/libconfig.h"   "$OUTPUT_FOLDER/include"
cp -f    "$aLibRoot/lib/libconfig.h++" "$OUTPUT_FOLDER/include"
echo "Output directory: $OUTPUT_FOLDER"

# armv7a
export "PATH=$HOME/develop/android15-armv7a/bin:$aPathBak"
export "CC=arm-linux-androideabi-gcc"
export "CXX=arm-linux-androideabi-g++"
export "CFLAGS=$aCFlagsArmv7a"
export "CXXFLAGS=$aCFlagsArmv7a"
./configure --host arm-linux-androideabi 2>&1 | tee $OUTPUT_FOLDER/config-armv7a.log
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
make clean
make -j$aNbJobs
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
#cp -f "$aLibRoot/lib/.libs/libconfig++.so" "$OUTPUT_FOLDER/libs/armeabi-v7a"
cp -f "$aLibRoot/lib/.libs/libconfig.a"   "$OUTPUT_FOLDER/libs/armeabi-v7a"
cp -f "$aLibRoot/lib/.libs/libconfig++.a" "$OUTPUT_FOLDER/libs/armeabi-v7a"

# x86
export "PATH=$HOME/develop/android15-x86/bin:$aPathBak"
export "CC=i686-linux-android-gcc"
export "CXX=i686-linux-android-g++"
export "CFLAGS=$aCFlagsx86"
export "CXXFLAGS=$aCFlagsx86"
./configure --host i686-linux-android 2>&1 | tee $OUTPUT_FOLDER/config-x86.log
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
make clean
make -j$aNbJobs
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
#cp -f "$aLibRoot/lib/.libs/libconfig++.so" "$OUTPUT_FOLDER/libs/x86"
cp -f "$aLibRoot/lib/.libs/libconfig.a"   "$OUTPUT_FOLDER/libs/x86"
cp -f "$aLibRoot/lib/.libs/libconfig++.a" "$OUTPUT_FOLDER/libs/x86"

# armv8a
export "PATH=$HOME/develop/android21-aarch64/bin:$aPathBak"
export "CC=aarch64-linux-android-gcc"
export "CXX=aarch64-linux-android-g++"
export "CFLAGS=$aCFlagsArmv8a"
export "CXXFLAGS=$aCFlagsArmv8a"
./configure --host aarch64-linux-android 2>&1 | tee $OUTPUT_FOLDER/config-aarch64.log
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
make clean
make -j$aNbJobs
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
#cp -f "$aLibRoot/lib/.libs/libconfig++.so" "$OUTPUT_FOLDER/libs/arm64-v8a"
cp -f "$aLibRoot/lib/.libs/libconfig.a"   "$OUTPUT_FOLDER/libs/arm64-v8a"
cp -f "$aLibRoot/lib/.libs/libconfig++.a" "$OUTPUT_FOLDER/libs/arm64-v8a"

export "PATH=$aPathBak"

rm $OUTPUT_FOLDER/../libconfig-android.7z &>/dev/null
7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on $OUTPUT_FOLDER/../libconfig-android.7z $OUTPUT_FOLDER
