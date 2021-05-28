#!/bin/bash

# This script generates the DEB and RPM packages.
#
# Copyright © Kirill Gavrilov, 2010-2013

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}
if [ -d "$aScriptPath" ]; then
    cd "$aScriptPath"
fi

aSuit=stable
for i in $*
do
  if [ "$i" == "ST_ALPHA" ] || [ "$i" == "ST_RELEASE_CANDIDATE" ]; then
    aSuit=experimental
  fi
done

# detect architecture
aSystem=`uname -s`
aSysVer=`lsb_release -s -c`
debArch=`uname -m`
rpmArch=`uname -m`
if [ "$debArch" != "x86_64" ] && [ "$debArch" != "ia64" ]; then
    debArch=i386
    rpmArch=i386
else
    debArch=amd64
    rpmArch=x86_64
fi

aDebRoot=temp/sView-$aSystem-$debArch

# prepare directories
rm --force --recursive $aDebRoot
mkdir -p $aDebRoot/debian
mkdir -p $aDebRoot/usr/bin
mkdir -p $aDebRoot/usr/lib
mkdir -p $aDebRoot/usr/share
mkdir -p $aDebRoot/usr/lib/sView/info
mkdir -p $aDebRoot/usr/lib/firefox/plugins
mkdir -p $aDebRoot/usr/lib/mozilla/plugins

# copy common files
cp -f -r ../share/*     $aDebRoot/usr/share/
cp -f    ../docs/license-gpl-3.0.txt $aDebRoot/usr/share/sView/info/license.txt

# copy sView compiled files
mv -f ../bin/LINUX_gcc/build.log $aDebRoot/
cp -f -r ../bin/LINUX_gcc/* $aDebRoot/usr/lib/sView/

# create symbolic links
ln --force --symbolic /usr/lib/sView/sView                                 $aDebRoot/usr/bin/sView
ln --force --symbolic /usr/lib/sView/StBrowserPlugins/npStBrowserPlugin.so $aDebRoot/usr/lib/firefox/plugins/npStBrowserPlugin.so
ln --force --symbolic /usr/lib/sView/StBrowserPlugins/npStBrowserPlugin.so $aDebRoot/usr/lib/mozilla/plugins/npStBrowserPlugin.so
ln --force --symbolic ../../share/sView/demo/demo.jps                      $aDebRoot/usr/lib/sView/demo.jps

# create DEB control file
debControlFile=$aDebRoot/debian/control

YEAR=$(date +"%y")
MONTH=$(date +"%m")
DAY=$(date +"%d")
debVersion=$YEAR.$MONTH.$DAY-1

cat > $debControlFile << EOF
Source:       sview.ru
Package:      sView
Version:      $debVersion
Section:      non-free
Priority:     extra
Architecture: $debArch
Depends:      libc6 (>= 2.4), libstdc++6 (>= 4.2.1), libgcc1 (>= 1:4.1.1), \
libavcodec52 (>= 3:0.svn20090303-1) | libavcodec-extra-52 (>= 3:0.svn20090303-1), libavformat52 (>= 3:0.svn20090303-1) | libavformat-extra-52 (>= 3:0.svn20090303-1), \
libswscale0 (>= 3:0.svn20090303-1) | libswscale-extra-0 (>= 3:0.svn20090303-1), \
libdevil1c2, \
libconfig++8, \
libfontconfig1 (>= 2.4.0), libfreetype6 (>= 2.2.1), libftgl2 (>= 2.1.3~rc5), \
libgl1-mesa-glx | libgl1, libglew1.5 (>= 1.5.1), \
libglib2.0-0 (>= 2.20.0), libgtk2.0-0 (>= 2.8.0), libatk1.0-0 (>= 1.20.0), libcairo2 (>= 1.2.4), libpango1.0-0 (>= 1.14.0), \
libopenal1
Maintainer:   Kirill Gavrilov <kirill@sview.ru>
Description:  sView
 .
 sView is a stereoscopic Image Viewer and Movie Player.
 Use OpenGL for rendering (requires hardware accelerated OpenGL2.0+ with GLSL1.1+)
 and OpenAL for sound output.
EOF

# detect dependencies
#cd $aDebRoot
#sViewBin=$PWD/usr/share/sView
#results=$PWD/debian/substvars
#dpkg-shlibdeps -O $sViewBin/StDrawers/StImageViewer.so $sViewBin/StDrawers/StMoviePlayer.so $sViewBin/sView $sViewBin/StCore.so $sViewBin/StMonitorsDump.so $sViewBin/StBrowserPlugins/npStBrowserPlugin.so $sViewBin/StRenderers/StOutAnaglyph.so $sViewBin/StRenderers/StOutDual.so $sViewBin/StRenderers/StOutInterlace.so $sViewBin/StRenderers/StOutIZ3D.so $sViewBin/StRenderers/StOutPageFlip.so
#cd ..

# prepare directory for dpkg --build
mv $aDebRoot/debian $aDebRoot/DEBIAN

# generate DEB-package
debTargetFile=sView\_$debVersion\_$debArch.deb
rpmTargetFile=sView-$debVersion.$rpmArch.rpm
rm -f $debTargetFile
rm -f $rpmTargetFile
dpkg --build $aDebRoot $debTargetFile

# convert DEB package to RPM package
alien --keep-version --to-rpm $debTargetFile

# prepare directories
mkdir -p repository/deb/pool/${aSysVer}-${aSuit}/non-free-${debArch}/s/sview
mkdir -p repository/rpm/pool/${aSysVer}-${aSuit}/non-free-${debArch}

# move the packages
mv -f $debTargetFile repository/deb/pool/${aSysVer}-${aSuit}/non-free-${debArch}/s/sview/
mv -f $rpmTargetFile repository/rpm/pool/${aSysVer}-${aSuit}/non-free-${debArch}/
