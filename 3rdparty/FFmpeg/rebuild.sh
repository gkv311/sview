#!/bin/bash

# This is helpful script to perform building of FFmpeg

echo "Usage: $0 [FFmpegPath] [GPL || LGPL] [DEBUG || RELEASE] [GCC_PREFIX]"

rebuildTarget="FFmpeg"
rebuildLicense="LGPL"
rebuildDebug="false"
compilerPrefix=""

for i in $*
do
  if [ "$i" == "gpl"       ] || [ "$i" == "GPL" ]; then
    rebuildLicense="GPL"
  elif [ "$i" == "lgpl"    ] || [ "$i" == "LGPL" ]; then
    rebuildLicense="LGPL"
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

# cross-compilers prefixes
GCC_MACHINE_MINGW_32="i686-w64-mingw32"
GCC_MACHINE_MINGW_32_1="i686-mingw32"
GCC_MACHINE_MINGW_64="x86_64-w64-mingw32"
GCC_MACHINE_MINGW_64_1="x86_64-pc-mingw32"
#GCC_MACHINE_LINUX_32=mingw32
GCC_MACHINE_LINUX_64="x86_64-linux-gnu"

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
rm -f -r $OUTPUT_FOLDER
mkdir -p $OUTPUT_FOLDER
mkdir -p $OUTPUT_FOLDER/bin
mkdir -p $OUTPUT_FOLDER/lib

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

configArguments="\
    --extra-version=sView.ru  \
    --enable-swscale          \
    --enable-shared           \
    --disable-static          \
    --enable-memalign-hack    \
    --enable-avfilter         \
    --enable-hardcoded-tables \
    --enable-pthreads         \
    --disable-libopenjpeg     \
    --enable-runtime-cpudetect"

#if [ "$gccMachine" != "$GCC_MACHINE_LINUX_64" ]; then
if [ "$gccMachine" == "$GCC_MACHINE_MINGW_32" ] || [ "$gccMachine" == "$GCC_MACHINE_MINGW_32_1" ] \
|| [ "$gccMachine" == "$GCC_MACHINE_MINGW_64" ] || [ "$gccMachine" == "$GCC_MACHINE_MINGW_64_1" ]; then
  # WinAPI threads are used only partially in FFmpeg
  # you should use with --enable-pthreads instead to enable full multithreading support!
  #configArguments="$configArguments --enable-w32threads"
  configArguments="$configArguments --enable-avisynth"
fi

# cross-compiling options
if [ "$gccMachine" == "$GCC_MACHINE_MINGW_32" ] || [ "$gccMachine" == "$GCC_MACHINE_MINGW_32_1" ]; then
  targetFlags="--cross-prefix=$compilerPrefix --arch=x86_32"
  configArguments="$configArguments --enable-cross-compile --target-os=mingw32 $targetFlags"
elif [ "$gccMachine" == "$GCC_MACHINE_MINGW_64" ] || [ "$gccMachine" == "$GCC_MACHINE_MINGW_64_1" ]; then
  targetFlags="--cross-prefix=$compilerPrefix --arch=x86_64 --extra-cflags=-Dstrtod=__strtod"
  configArguments="$configArguments --enable-cross-compile --target-os=mingw32 $targetFlags"
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

echo
echo "  ./configure $configArguments"
echo
./configure $configArguments >$OUTPUT_FOLDER/config.log 2>&1
cat $OUTPUT_FOLDER/config.log
echo

make -j6 2>$OUTPUT_FOLDER/make.log
cat $OUTPUT_FOLDER/make.log | grep -i -w 'ошибка'
echo

# Now copy result files
echo "  Now copy result files into $OUTPUT_FOLDER"

"$compilerPrefix"gcc --version > $OUTPUT_FOLDER/gccInfo.log

if [ -f libavcodec/avcodec.dll ]; then
  cp -f libavcodec/*.dll $OUTPUT_FOLDER
  cp -f libavcodec/*.lib $OUTPUT_FOLDER &>/dev/null
elif [ -f libavcodec/libavcodec.dylib ]; then
  cp -f -d libavcodec/*.dylib* $OUTPUT_FOLDER/lib
else
  cp -f -d libavcodec/*.so* $OUTPUT_FOLDER/lib
fi

if [ -f libavdevice/avdevice.dll ]; then
  cp -f libavdevice/*.dll $OUTPUT_FOLDER
  cp -f libavdevice/*.lib $OUTPUT_FOLDER &>/dev/null
elif [ -f libavdevice/libavdevice.dylib ]; then
  cp -f -d libavdevice/*.dylib* $OUTPUT_FOLDER/lib
else
  cp -f -d libavdevice/*.so* $OUTPUT_FOLDER/lib
fi

if [ -f libavfilter/avfilter.dll ]; then
  cp -f libavfilter/*.dll $OUTPUT_FOLDER
  cp -f libavfilter/*.lib $OUTPUT_FOLDER &>/dev/null
elif [ -f libavfilter/libavfilter.dylib ]; then
  cp -f -d libavfilter/*.dylib* $OUTPUT_FOLDER/lib
else
  cp -f -d libavfilter/*.so* $OUTPUT_FOLDER/lib
fi

if [ -f libavformat/avformat.dll ]; then
  cp -f libavformat/*.dll $OUTPUT_FOLDER
  cp -f libavformat/*.lib $OUTPUT_FOLDER &>/dev/null
elif [ -f libavformat/libavformat.dylib ]; then
  cp -f -d libavformat/*.dylib* $OUTPUT_FOLDER/lib
else
  cp -f -d libavformat/*.so* $OUTPUT_FOLDER/lib
fi

if [ -f libavutil/avutil.dll ]; then
  cp -f libavutil/*.dll $OUTPUT_FOLDER
  cp -f libavutil/*.lib $OUTPUT_FOLDER &>/dev/null
elif [ -f libavutil/libavutil.dylib ]; then
  cp -f -d libavutil/*.dylib* $OUTPUT_FOLDER/lib
else
  cp -f -d libavutil/*.so* $OUTPUT_FOLDER/lib
fi

if [ -f libswscale/swscale.dll ]; then
  cp -f libswscale/*.dll $OUTPUT_FOLDER
  cp -f libswscale/*.lib $OUTPUT_FOLDER &>/dev/null
elif [ -f libswscale/libswscale.dylib ]; then
  cp -f -d libswscale/*.dylib* $OUTPUT_FOLDER/lib
else
  cp -f -d libswscale/*.so* $OUTPUT_FOLDER/lib
fi

if [ -f libswresample/swresample.dll ]; then
  cp -f libswresample/*.dll $OUTPUT_FOLDER
  cp -f libswresample/*.lib $OUTPUT_FOLDER &>/dev/null
elif [ -f libswresample/libswresample.dylib ]; then
  cp -f -d libswresample/*.dylib* $OUTPUT_FOLDER/lib
else
  cp -f -d libswresample/*.so* $OUTPUT_FOLDER/lib
fi

cp -f *.exe      $OUTPUT_FOLDER &>/dev/null
cp -f ffmpeg     $OUTPUT_FOLDER/bin &>/dev/null
cp -f ffmpeg_g   $OUTPUT_FOLDER/bin &>/dev/null
cp -f ffprobe    $OUTPUT_FOLDER/bin &>/dev/null
cp -f ffprobe_g  $OUTPUT_FOLDER/bin &>/dev/null
cp -f ffserver   $OUTPUT_FOLDER/bin &>/dev/null
cp -f ffserver_g $OUTPUT_FOLDER/bin &>/dev/null
cp -f ffplay     $OUTPUT_FOLDER/bin &>/dev/null

# remove duplicates (only Windows)
rm $OUTPUT_FOLDER/avcodec.dll $OUTPUT_FOLDER/swresample.dll $OUTPUT_FOLDER/avdevice.dll $OUTPUT_FOLDER/avfilter.dll $OUTPUT_FOLDER/avformat.dll $OUTPUT_FOLDER/avutil.dll $OUTPUT_FOLDER/swscale.dll &>/dev/null

# create binaries archive
if command -v 7za &>/dev/null
then
  # binaries
  rm $OUTPUT_FOLDER/../$OUTPUT_NAME.7z &>/dev/null
  7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on $OUTPUT_FOLDER/../$OUTPUT_NAME.7z $OUTPUT_FOLDER
  rm -f -r $OUTPUT_FOLDER
fi

# come back
cd ..
