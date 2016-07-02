#!/bin/bash

# This is helpful script to perform building of FFmpeg

echo "Usage: $0 [FFmpegPath] [GPL || LGPL] [DEBUG || RELEASE] [GCC_PREFIX]"

rebuildTarget="FFmpeg"
rebuildLicense="LGPL"
rebuildDebug="false"
rebuildAndroid="false"
compilerPrefix=""
androidTarget=""
androidNdkRoot="$HOME/develop/android-ndk-r12b"
aSystem=`uname -s`
aPwdBack=$PWD

for i in $*
do
  if [ "$i" == "gpl"       ] || [ "$i" == "GPL" ]; then
    rebuildLicense="GPL"
  elif [ "$i" == "lgpl"    ] || [ "$i" == "LGPL" ]; then
    rebuildLicense="LGPL"
  elif [ "$i" == "android" ]; then
    rebuildAndroid="true"
  elif [ "$i" == "debug"   ] || [ "$i" == "DEBUG" ]; then
    rebuildDebug="true"
  elif [ "$i" == "release" ] || [ "$i" == "RELEASE" ]; then
    rebuildDebug="false"
  elif [ -d "$i" ]; then
    rebuildTarget="${i}"
  elif [ "$i" != "" ]; then
    compilerPrefix="${i}"
  fi
done

if [ "$rebuildAndroid" == "true" ]; then
  if [ "$androidTarget" == "" ]; then
    androidTarget="arm"
    #androidTarget="arm64"
  fi
fi

# cross-compilers prefixes
GCC_MACHINE_MINGW_32="i686-w64-mingw32"
GCC_MACHINE_MINGW_32_1="i686-mingw32"
GCC_MACHINE_MINGW_64="x86_64-w64-mingw32"
GCC_MACHINE_MINGW_64_1="x86_64-pc-mingw32"
#GCC_MACHINE_LINUX_32=mingw32
GCC_MACHINE_LINUX_64="x86_64-linux-gnu"

if [ "$rebuildAndroid" == "true" ]; then
  if [ "$androidTarget" == "arm64" ]; then
    aPrefixShort="aarch64-linux-android"
  else
    aPrefixShort="arm-linux-androideabi"
  fi

  compilerPrefix="${androidNdkRoot}/toolchains/${aPrefixShort}-4.8/prebuilt/linux-x86_64/bin/${aPrefixShort}-"
  if [ -x "${androidNdkRoot}/toolchains/${aPrefixShort}-4.9" ]; then
    compilerPrefix="${androidNdkRoot}/toolchains/${aPrefixShort}-4.9/prebuilt/linux-x86_64/bin/${aPrefixShort}-"
  fi
fi

if [ "$compilerPrefix" != "" ]; then
  gccVersion=$("$compilerPrefix"gcc -dumpversion)
  gccMachine=$("$compilerPrefix"gcc -dumpmachine)
else
  gccVersion=$(gcc -dumpversion)
  gccMachine=$(gcc -dumpmachine)
fi

echo "  Start building FFmpeg $rebuildLicense from $rebuildTarget"
cd $rebuildTarget

# releases extract the version number from the VERSION file
ffmpegVersion=$(cat VERSION 2> /dev/null)
ffmpegRevision=$(git log -1 --pretty=format:%h 2> /dev/null)
ffmpegDate=$(git show -s --format="%ci" 2> /dev/null)
ffmpegDate=${ffmpegDate:0:10}
test $ffmpegRevision && ffmpegRevision=git-$ffmpegRevision
test $ffmpegVersion || ffmpegVersion=$ffmpegRevision

# remove slashes
rebuildTarget="${rebuildTarget//\//}"
SOURCES_NAME="FFmpeg-$ffmpegDate-$ffmpegVersion"
OUTPUT_NAME="$SOURCES_NAME-$gccMachine-$gccVersion-$rebuildLicense"
SOURCES_NAME="$SOURCES_NAME-src"
if [ "$rebuildDebug" == "true" ]; then
  OUTPUT_NAME="$OUTPUT_NAME-debug"
fi
OUTPUT_FOLDER="../$OUTPUT_NAME"
OUTPUT_FOLDER_INC="$OUTPUT_FOLDER/include"
OUTPUT_FOLDER_BIN="$OUTPUT_FOLDER/bin"
OUTPUT_FOLDER_LIB="$OUTPUT_FOLDER/lib"
if [ "$aSystem" == "Darwin" ]; then
  OUTPUT_FOLDER_BIN="$OUTPUT_FOLDER/MacOS"
  OUTPUT_FOLDER_LIB="$OUTPUT_FOLDER/Frameworks"
fi
if [ "$androidTarget" == "arm64" ]; then
  OUTPUT_FOLDER_LIB="$OUTPUT_FOLDER/libs/arm64-v8a"
elif [ "$androidTarget" == "arm" ]; then
  OUTPUT_FOLDER_LIB="$OUTPUT_FOLDER/libs/armeabi-v7a"
fi

rm -f -r $OUTPUT_FOLDER
mkdir -p $OUTPUT_FOLDER
mkdir -p $OUTPUT_FOLDER_BIN
mkdir -p $OUTPUT_FOLDER_LIB
mkdir -p $OUTPUT_FOLDER_INC
mkdir -p $OUTPUT_FOLDER_INC/libavcodec
mkdir -p $OUTPUT_FOLDER_INC/libavdevice
mkdir -p $OUTPUT_FOLDER_INC/libavfilter
mkdir -p $OUTPUT_FOLDER_INC/libavformat
mkdir -p $OUTPUT_FOLDER_INC/libavutil
mkdir -p $OUTPUT_FOLDER_INC/libswscale
mkdir -p $OUTPUT_FOLDER_INC/libswresample

echo "  make distclean"
make distclean &>/dev/null

# create sources archive
if command -v 7za &>/dev/null
then
  if [ -f ../$SOURCES_NAME.7z ]
  then
    echo "Sources archive '$SOURCES_NAME.7z' already exists"
  else
    7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on ../$SOURCES_NAME.7z ../$rebuildTarget '-xr@../exclude.lst'
  fi
fi

#--enable-memalign-hack
configArguments="\
 --extra-version=sView.ru \
 --enable-swscale \
 --enable-shared \
 --disable-static \
 --enable-avfilter \
 --enable-hardcoded-tables \
 --enable-pthreads \
 --disable-libopenjpeg \
 --disable-doc \
 --enable-runtime-cpudetect"

if [ "$aSystem" == "Darwin" ]; then
  configArguments="$configArguments --disable-audiotoolbox --disable-hwaccel=xvmc"
  configArguments="$configArguments --enable-vda --libdir=@executable_path/../Frameworks"
fi

#if [ "$gccMachine" != "$GCC_MACHINE_LINUX_64" ]; then
if [ "$gccMachine" == "$GCC_MACHINE_MINGW_32" ] || [ "$gccMachine" == "$GCC_MACHINE_MINGW_32_1" ] \
|| [ "$gccMachine" == "$GCC_MACHINE_MINGW_64" ] || [ "$gccMachine" == "$GCC_MACHINE_MINGW_64_1" ]; then
  # WinAPI threads are used only partially in FFmpeg
  # you should use with --enable-pthreads instead to enable full multithreading support!
  #configArguments="$configArguments --enable-w32threads"
  configArguments="$configArguments --enable-avisynth"

  # avoid dynamic linkage with libgcc_s_sjlj-1.dll
  configArguments="$configArguments --extra-ldflags=-static-libgcc"
fi

# cross-compiling options
if [ "$gccMachine" == "$GCC_MACHINE_MINGW_32" ] || [ "$gccMachine" == "$GCC_MACHINE_MINGW_32_1" ]; then
  targetFlags="--cross-prefix=$compilerPrefix --arch=x86_32"
  configArguments="$configArguments --enable-cross-compile --target-os=mingw32 $targetFlags"
elif [ "$gccMachine" == "$GCC_MACHINE_MINGW_64" ] || [ "$gccMachine" == "$GCC_MACHINE_MINGW_64_1" ]; then
  targetFlags="--cross-prefix=$compilerPrefix --arch=x86_64 --extra-cflags=-Dstrtod=__strtod"
  configArguments="$configArguments --enable-cross-compile --target-os=mingw32 $targetFlags"
elif [ "$rebuildAndroid" == "true" ]; then
  if [ "$androidTarget" == "arm64" ]; then
    targetFlags="--cross-prefix=$compilerPrefix --sysroot=${androidNdkRoot}/platforms/android-21/arch-arm64 --arch=aarch64"
  else
    targetFlags="--cross-prefix=$compilerPrefix --sysroot=${androidNdkRoot}/platforms/android-15/arch-arm --arch=arm"
  fi

  configArguments="$configArguments --enable-cross-compile --target-os=linux $targetFlags"
  #configArguments="$configArguments --extra-cflags='-fno-builtin-sin -fno-builtin-sinf'"

  configArguments="$configArguments --enable-jni --enable-mediacodec"
fi

# More options
#configArguments="$configArguments --enable-libvpx
#--enable-libfaac --enable-libmp3lame --enable-libgsm --enable-libspeex --enable-libtheora --enable-libvorbis --enable-libx264 --enable-libxvid

if [ "$rebuildDebug" == "true" ]; then
  #disable stripping of executables and shared libraries
  configArguments="$configArguments --disable-stripping --disable-optimizations --enable-debug=3"
  #configArguments="$configArguments --extra-cflags=-fno-inline" #configArguments="$configArguments --extra-cflags=\"-O0 -fno-inline\""
else
  #disable debugging symbols
  configArguments="$configArguments --disable-debug"
fi

# enable (L)GPL version 3
configArguments="$configArguments --enable-version3"

if [ "$rebuildLicense" == "GPL" ]; then
  configArguments="$configArguments --enable-gpl"
  # enable libx264 encoder (should be compiled and installed!)
  #configArguments="$configArguments --enable-libx264"
  # enable GPLed postprocessing support, what for???
  #configArguments="$configArguments --enable-postproc"
fi

# redirect error state from tee
set -i pipefail

aNbJobs="$(getconf _NPROCESSORS_ONLN)"

echo
echo "  ./configure $configArguments"
echo
if [ "$rebuildAndroid" == "true" ]; then
  if [ "$androidTarget" == "arm64" ]; then
    ./configure $configArguments --disable-symver --extra-cflags='-march=armv8-a' 2>&1 | tee $OUTPUT_FOLDER/config.log
  else
    ./configure $configArguments --disable-symver --extra-cflags='-march=armv7-a -mfloat-abi=softfp -fno-builtin-sin -fno-builtin-sinf' 2>&1 | tee $OUTPUT_FOLDER/config.log
  fi
else
  ./configure $configArguments 2>&1 | tee -a $OUTPUT_FOLDER/config.log
fi
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi

make -j $aNbJobs 2>&1 | tee -a $OUTPUT_FOLDER/make.log
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi

# Now copy result files
echo
echo "  Now copy result files into $OUTPUT_FOLDER"

"$compilerPrefix"gcc --version > $OUTPUT_FOLDER/gccInfo.log

if [ -f libavcodec/avcodec.dll ]; then
  cp -f libavcodec/*.dll $OUTPUT_FOLDER_BIN
  cp -f libavcodec/*.lib $OUTPUT_FOLDER_LIB &>/dev/null
elif [ -f libavcodec/libavcodec.dylib ]; then
  cp -f -p -R libavcodec/*.dylib* $OUTPUT_FOLDER_LIB
else
  cp -f -d libavcodec/*.so* $OUTPUT_FOLDER_LIB
fi

if [ -f libavdevice/avdevice.dll ]; then
  cp -f libavdevice/*.dll $OUTPUT_FOLDER_BIN
  cp -f libavdevice/*.lib $OUTPUT_FOLDER_LIB &>/dev/null
elif [ -f libavdevice/libavdevice.dylib ]; then
  cp -f -p -R libavdevice/*.dylib* $OUTPUT_FOLDER_LIB
else
  cp -f -d libavdevice/*.so* $OUTPUT_FOLDER_LIB
fi

if [ -f libavfilter/avfilter.dll ]; then
  cp -f libavfilter/*.dll $OUTPUT_FOLDER_BIN
  cp -f libavfilter/*.lib $OUTPUT_FOLDER_LIB &>/dev/null
elif [ -f libavfilter/libavfilter.dylib ]; then
  cp -f -p -R libavfilter/*.dylib* $OUTPUT_FOLDER_LIB
else
  cp -f -d libavfilter/*.so* $OUTPUT_FOLDER_LIB
fi

if [ -f libavformat/avformat.dll ]; then
  cp -f libavformat/*.dll $OUTPUT_FOLDER_BIN
  cp -f libavformat/*.lib $OUTPUT_FOLDER_LIB &>/dev/null
elif [ -f libavformat/libavformat.dylib ]; then
  cp -f -p -R libavformat/*.dylib* $OUTPUT_FOLDER_LIB
else
  cp -f -d libavformat/*.so* $OUTPUT_FOLDER_LIB
fi

if [ -f libavutil/avutil.dll ]; then
  cp -f libavutil/*.dll $OUTPUT_FOLDER_BIN
  cp -f libavutil/*.lib $OUTPUT_FOLDER_LIB &>/dev/null
elif [ -f libavutil/libavutil.dylib ]; then
  cp -f -p -R libavutil/*.dylib* $OUTPUT_FOLDER_LIB
else
  cp -f -d libavutil/*.so* $OUTPUT_FOLDER_LIB
fi

if [ -f libswscale/swscale.dll ]; then
  cp -f libswscale/*.dll $OUTPUT_FOLDER_BIN
  cp -f libswscale/*.lib $OUTPUT_FOLDER_LIB &>/dev/null
elif [ -f libswscale/libswscale.dylib ]; then
  cp -f -p -R libswscale/*.dylib* $OUTPUT_FOLDER_LIB
else
  cp -f -d libswscale/*.so* $OUTPUT_FOLDER_LIB
fi

if [ -f libswresample/swresample.dll ]; then
  cp -f libswresample/*.dll $OUTPUT_FOLDER_BIN
  cp -f libswresample/*.lib $OUTPUT_FOLDER_LIB &>/dev/null
elif [ -f libswresample/libswresample.dylib ]; then
  cp -f -p -R libswresample/*.dylib* $OUTPUT_FOLDER_LIB
else
  cp -f -d libswresample/*.so* $OUTPUT_FOLDER_LIB
fi

cp -f *.exe      $OUTPUT_FOLDER_BIN &>/dev/null
cp -f ffmpeg     $OUTPUT_FOLDER_BIN &>/dev/null
cp -f ffmpeg_g   $OUTPUT_FOLDER_BIN &>/dev/null
cp -f ffprobe    $OUTPUT_FOLDER_BIN &>/dev/null
cp -f ffprobe_g  $OUTPUT_FOLDER_BIN &>/dev/null
cp -f ffserver   $OUTPUT_FOLDER_BIN &>/dev/null
cp -f ffserver_g $OUTPUT_FOLDER_BIN &>/dev/null
cp -f ffplay     $OUTPUT_FOLDER_BIN &>/dev/null

cp -f libavcodec/*.h    $OUTPUT_FOLDER_INC/libavcodec    &>/dev/null
cp -f libavdevice/*.h   $OUTPUT_FOLDER_INC/libavdevice   &>/dev/null
cp -f libavfilter/*.h   $OUTPUT_FOLDER_INC/libavfilter   &>/dev/null
cp -f libavformat/*.h   $OUTPUT_FOLDER_INC/libavformat   &>/dev/null
cp -f libavutil/*.h     $OUTPUT_FOLDER_INC/libavutil     &>/dev/null
cp -f libswscale/*.h    $OUTPUT_FOLDER_INC/libswscale    &>/dev/null
cp -f libswresample/*.h $OUTPUT_FOLDER_INC/libswresample &>/dev/null

# remove duplicates (only Windows)
rm $OUTPUT_FOLDER_BIN/avcodec.dll $OUTPUT_FOLDER_BIN/swresample.dll $OUTPUT_FOLDER_BIN/avdevice.dll $OUTPUT_FOLDER_BIN/avfilter.dll $OUTPUT_FOLDER_BIN/avformat.dll $OUTPUT_FOLDER_BIN/avutil.dll $OUTPUT_FOLDER_BIN/swscale.dll &>/dev/null

# create binaries archive
if [ "$aSystem" == "Darwin" ]; then
  rm $OUTPUT_FOLDER/../$OUTPUT_NAME.tar.gz &>/dev/null
  cd $OUTPUT_FOLDER
  tar -cvzf $OUTPUT_FOLDER/../$OUTPUT_NAME.tar.gz *
elif command -v 7za &>/dev/null
then
  rm $OUTPUT_FOLDER/../$OUTPUT_NAME.7z &>/dev/null
  7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on $OUTPUT_FOLDER/../$OUTPUT_NAME.7z $OUTPUT_FOLDER
fi

# come back
cd $aPwdBack
#rm -f -r $OUTPUT_FOLDER
