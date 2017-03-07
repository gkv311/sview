#!/bin/bash

# small help to build FreeType for Android [for sView project]
# https://www.freetype.org

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
aCPrefixArmv7a="arm-linux-androideabi-"
aCPrefixArmv8a="aarch64-linux-android-"
aCPrefixx86="i686-linux-android-"

OUTPUT_FOLDER="$aLibRoot/install/freetype-android"
rm -f -r "$OUTPUT_FOLDER"
mkdir -p "$OUTPUT_FOLDER/include"
mkdir -p "$OUTPUT_FOLDER/libs/armeabi-v7a"
mkdir -p "$OUTPUT_FOLDER/libs/x86"
mkdir -p "$OUTPUT_FOLDER/libs/arm64-v8a"
cp -f    "$aLibRoot/README"         "$OUTPUT_FOLDER"
cp -f    "$aLibRoot/docs/FTL.TXT"   "$OUTPUT_FOLDER"
cp -f    "$aLibRoot/docs/GPLv2.TXT" "$OUTPUT_FOLDER"
cp -f    "$aLibRoot/docs/CHANGES"   "$OUTPUT_FOLDER"
cp -f -r "$aLibRoot/include"        "$OUTPUT_FOLDER"
rm -f -r "$OUTPUT_FOLDER/include/freetype/internal"
echo "Output directory: $OUTPUT_FOLDER"

# armv7a
export "PATH=$HOME/develop/android15-armv7a/bin:$aPathBak"
export "CC=${aCPrefixArmv7a}gcc"
export "CXX=${aCPrefixArmv7a}g++"
export "CFLAGS=$aCFlagsArmv7a"
./configure --host arm-linux-androideabi --enable-shared --with-png=no --with-harfbuzz=no 2>&1 | tee $OUTPUT_FOLDER/config-armv7a.log
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
make clean
make -j$aNbJobs
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
cp -f "$aLibRoot/objs/.libs/libfreetype.so" "$OUTPUT_FOLDER/libs/armeabi-v7a"
${aCPrefixArmv7a}strip --strip-unneeded     "$OUTPUT_FOLDER/libs/armeabi-v7a/libfreetype.so"
#cp -f "$aLibRoot/objs/.libs/libfreetype.a" "$OUTPUT_FOLDER/libs/armeabi-v7a"

# x86
export "PATH=$HOME/develop/android15-x86/bin:$aPathBak"
export "CC=${aCPrefixx86}gcc"
export "CXX=${aCPrefixx86}g++"
export "CFLAGS=$aCFlagsx86"
./configure --host i686-linux-android --enable-shared --with-png=no --with-harfbuzz=no 2>&1 | tee $OUTPUT_FOLDER/config-x86.log
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
make clean
make -j$aNbJobs
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
cp -f "$aLibRoot/objs/.libs/libfreetype.so" "$OUTPUT_FOLDER/libs/x86"
${aCPrefixx86}strip --strip-unneeded        "$OUTPUT_FOLDER/libs/x86/libfreetype.so"
#cp -f "$aLibRoot/objs/.libs/libfreetype.a" "$OUTPUT_FOLDER/libs/x86"

# armv8a
export "PATH=$HOME/develop/android21-aarch64/bin:$aPathBak"
export "CC=${aCPrefixArmv8a}gcc"
export "CXX=${aCPrefixArmv8a}g++"
export "CFLAGS=$aCFlagsArmv8a"
./configure --host aarch64-linux-android --enable-shared --with-png=no --with-harfbuzz=no 2>&1 | tee $OUTPUT_FOLDER/config-aarch64.log
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
make clean
make -j$aNbJobs
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
cp -f "$aLibRoot/objs/.libs/libfreetype.so" "$OUTPUT_FOLDER/libs/arm64-v8a"
${aCPrefixArmv8a}strip --strip-unneeded     "$OUTPUT_FOLDER/libs/arm64-v8a/libfreetype.so"
#cp -f "$aLibRoot/objs/.libs/libfreetype.a" "$OUTPUT_FOLDER/libs/arm64-v8a"

export "PATH=$aPathBak"

rm $OUTPUT_FOLDER/../freetype-android.7z &>/dev/null
7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on $OUTPUT_FOLDER/../freetype-android.7z $OUTPUT_FOLDER
