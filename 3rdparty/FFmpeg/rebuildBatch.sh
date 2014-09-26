#!/bin/bash

# This is helpful script to perform building of FFmpeg

# MinGW tools for Ubuntu; http://ffmpeg.arrozcru.org/wiki/index.php?title=Aptrepository
# Add gpg key to your system 
#$ gpg --keyserver keyserver.ubuntu.com --recv-key 0x25E635F9
#$ gpg --export --armor 0x25E635F9 | sudo apt-key add -
#deb http://apt.arrozcru.org ./
#deb-src http://apt.arrozcru.org ./

#sudo apt-get install mingw32-w32api mingw32-runtime mingw32-gcc-4.4 mingw32-binutils mingw32-zlib mingw32-bzip2 mingw32-pthreads 
#sudo apt-get install mingw-w64-w32api mingw-w64-runtime mingw-w64-gcc-4.4 mingw-w64-crt mingw-w64-binutils mingw-w64-headers mingw-w64-zlib mingw-w64-bzip2 mingw-w64-pthreads

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
#./rebuild.sh "$rebuildTarget" GPL  DEBUG   "i686-mingw32-"
./rebuild.sh "$rebuildTarget" LGPL DEBUG   "i686-mingw32-"
./rebuild.sh "$rebuildTarget" GPL  RELEASE "i686-mingw32-"
./rebuild.sh "$rebuildTarget" LGPL RELEASE "i686-mingw32-"

# build for Win32 AMD64
#./rebuild.sh "$rebuildTarget" GPL  DEBUG   "x86_64-w64-mingw32-"
#./rebuild.sh "$rebuildTarget" LGPL DEBUG   "x86_64-w64-mingw32-"
#./rebuild.sh "$rebuildTarget" GPL  RELEASE "x86_64-w64-mingw32-"
./rebuild.sh "$rebuildTarget" LGPL RELEASE "x86_64-w64-mingw32-"

# build for Linux
./rebuild.sh "$rebuildTarget" GPL  RELEASE
./rebuild.sh "$rebuildTarget" LGPL RELEASE

# build for Android
#./rebuild.sh "$rebuildTarget" GPL RELEASE "android"
