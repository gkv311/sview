#!/bin/bash

# This is helpful script to perform building of FFmpeg

# mingw for cross-compiling
#sudo apt-get install binutils-mingw-w64-i686 binutils-mingw-w64-x86-64 g++-mingw-w64-i686 g++-mingw-w64-x86-64 gcc-mingw-w64 gcc-mingw-w64-base gcc-mingw-w64-i686 gcc-mingw-w64-x86-64
#sudo apt-get install mingw-w64 mingw-w64-common mingw-w64-tools yasm p7zip-full git git-gui gitk

# avoid linkage with dynamic version of libwinpthread.dll
#sudo mv /usr/x86_64-w64-mingw32/lib/libpthread.dll.a /usr/x86_64-w64-mingw32/lib/__libpthread.dll.a
#sudo mv /usr/x86_64-w64-mingw32/lib/libwinpthread.dll.a /usr/x86_64-w64-mingw32/lib/__libwinpthread.dll.a
#sudo mv /usr/i686-w64-mingw32/lib/libpthread.dll.a /usr/i686-w64-mingw32/lib/__libpthread.dll.a
#sudo mv /usr/i686-w64-mingw32/lib/libwinpthread.dll.a /usr/i686-w64-mingw32/lib/__libwinpthread.dll.a

rebuildTarget="FFmpeg"
if [ "$1" != "" ]; then
    rebuildTarget="${1}"
else
  if command -v git &>/dev/null
  then
    # clone the main repository
    rm -f -r FFmpeg-src
    # clone the whole repository and checkout last successful revision...
    git clone git://git.videolan.org/ffmpeg.git FFmpeg-src
    cd FFmpeg-src
    git checkout 0cd1383
    cd ..
    # clone only HEAD revision, maybe broken...
    #git clone --depth 1 git://git.videolan.org/ffmpeg.git FFmpeg-src
    rebuildTarget="FFmpeg-src"
  else
    echo "You should install git before"
    return 1
  fi
fi

# build for Win32 x86
#./ffmpeg-rebuild.sh "$rebuildTarget" GPL  DEBUG   "i686-w64-mingw32-"
./ffmpeg-rebuild.sh "$rebuildTarget" LGPL DEBUG   "i686-w64-mingw32-"
./ffmpeg-rebuild.sh "$rebuildTarget" GPL  RELEASE "i686-w64-mingw32-"
./ffmpeg-rebuild.sh "$rebuildTarget" LGPL RELEASE "i686-w64-mingw32-"

# build for Win32 AMD64
#./ffmpeg-rebuild.sh "$rebuildTarget" GPL  DEBUG   "x86_64-w64-mingw32-"
#./ffmpeg-rebuild.sh "$rebuildTarget" LGPL DEBUG   "x86_64-w64-mingw32-"
#./ffmpeg-rebuild.sh "$rebuildTarget" GPL  RELEASE "x86_64-w64-mingw32-"
./ffmpeg-rebuild.sh "$rebuildTarget" LGPL RELEASE "x86_64-w64-mingw32-"

# build for Linux
./ffmpeg-rebuild.sh "$rebuildTarget" GPL  RELEASE
./ffmpeg-rebuild.sh "$rebuildTarget" LGPL RELEASE

# build for Android
#./ffmpeg-rebuild.sh "$rebuildTarget" GPL RELEASE "android"
