#!/bin/bash

# This script generates the DEB source package.
#
# Copyright © Kirill Gavrilov, 2012-2013

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}
if [ -d "$aScriptPath" ]; then
  cd "$aScriptPath"
fi

#sudo apt-get install build-essential devscripts

# build source package and sign it
#debuildParams="-S"
# build unsigned source package
#debuildParams="-S -us -uc"
# build unsigned source and binary packages
debuildParams="-us -uc"

aCurrentRelease=`lsb_release -c -s`
aDistribs=("$aCurrentRelease")

# Ubuntu 14.04 LTS (Trusty Tahr)
# Ubuntu 15.04     (Vivid Vervet)
# Ubuntu 15.10     (Wily Werewolf)
# Ubuntu 16.04 LTS (Xenial Xerus)
# Ubuntu 16.10     (Yakkety Yak)
# Ubuntu 17.04     (Zesty Zapus)
# Ubuntu 17.10     (Artful Aardvark)
# Ubuntu 18.04 LTS (Bionic Beaver)
# Ubuntu 18.10     (Cosmic Cuttlefish)
# Ubuntu 19.04     (Disco Dingo)
# Ubuntu 19.10
# Ubuntu 20.04 LTS (Focal Fossa)
# Ubuntu 20.10     (Groovy Gorilla)
# Ubuntu 21.04     (Hirsute Hippo)
# Ubuntu 21.10     (Impish Indri)
# Ubuntu 22.04 LTS (Jammy Jellyfish)
# Ubuntu 24.04 LTS (Noble Numbat)
#aDistribs=("focal" "jammy" "noble")

# Debian
#aDistribs=("stable" "unstable" "testing-proposed-updates" "experimental")

YEAR=$(date +"%y")
MONTH=$(date +"%m")
DAY=$(date +"%d")
aCurrentDate=`date --rfc-2822`
aVersion=${YEAR}.${MONTH}
aRelease=1
aDebVersion=${aVersion}-${aRelease}
aDebSrcRoot=temp/sview-${aDebVersion}

set -o pipefail

# copy common files
rm --force --recursive     $aDebSrcRoot
mkdir -p $aDebSrcRoot
cp -f -r ../debian         $aDebSrcRoot/
cp -f -r ../share          $aDebSrcRoot/
mkdir -p $aDebSrcRoot/distribution
cp -f -r ../distribution/info $aDebSrcRoot/distribution/
cp -f -r ../include        $aDebSrcRoot/
rm --force $aDebSrcRoot/include/stconfig.conf
cp -f -r ../StCore         $aDebSrcRoot/
cp -f -r ../StGLWidgets    $aDebSrcRoot/
cp -f -r ../StImageViewer  $aDebSrcRoot/
cp -f -r ../StMoviePlayer  $aDebSrcRoot/
cp -f -r ../StDiagnostics  $aDebSrcRoot/
cp -f -r ../StCADViewer    $aDebSrcRoot/
cp -f -r ../StOutAnaglyph  $aDebSrcRoot/
cp -f -r ../StOutDual      $aDebSrcRoot/
cp -f -r ../StOutInterlace $aDebSrcRoot/
cp -f -r ../StOutIZ3D      $aDebSrcRoot/
cp -f -r ../StOutPageFlip  $aDebSrcRoot/
cp -f -r ../StOutDistorted $aDebSrcRoot/
cp -f -r ../StShared       $aDebSrcRoot/
cp -f -r ../sview          $aDebSrcRoot/
cp -f -r ../textures       $aDebSrcRoot/
cp -f -r ../docs           $aDebSrcRoot/
#cp -f    ../Makefile       $aDebSrcRoot/
cp -f -r ../adm            $aDebSrcRoot/
cp -f    ../CMakeLists.txt $aDebSrcRoot/
cp -f -r ../StMonitorsDump $aDebSrcRoot/
cp -f -r ../StTests        $aDebSrcRoot/
cp -f    ../README.md      $aDebSrcRoot/
cp -f    ../README.md      $aDebSrcRoot/README
cp -f    ../LICENSE_GPL_3-0.txt $aDebSrcRoot/LICENSE

mkdir -p $aDebSrcRoot/3rdparty/include
cp -f -r ../3rdparty/include/adlsdk     $aDebSrcRoot/3rdparty/include/
cp -f -r ../3rdparty/include/gl         $aDebSrcRoot/3rdparty/include/
cp -f    ../3rdparty/include/mongoose.* $aDebSrcRoot/3rdparty/include/

find $aDebSrcRoot -name  "obj"      -exec rm -f -r {} \; 2>/dev/null
find $aDebSrcRoot -iname "*.tmp"    -exec rm {} \;
find $aDebSrcRoot -iname "*.bak"    -exec rm {} \;
find $aDebSrcRoot -iname "*.o"      -exec rm {} \;
find $aDebSrcRoot -iname "*.depend" -exec rm {} \;
find $aDebSrcRoot -iname "*.layout" -exec rm {} \;
find $aDebSrcRoot -iname "*.exp"    -exec rm {} \;
find $aDebSrcRoot -iname "*_build_log.html" -exec rm {} \;

sed "s/unknown_version/${aVersion}/g"  "sView.rpm.spec" > "$aDebSrcRoot/sView.rpm.spectmp1"
sed "s/unknown_release/${aRelease}/g"  "$aDebSrcRoot/sView.rpm.spectmp1" > "$aDebSrcRoot/sView.rpm.spec"
#sed "s/unknown_date/${aCurrentDate}/g" "$aDebSrcRoot/sView.rpm.spectmp2"  > "$aDebSrcRoot/sView.rpm.spec"
rm -f $aDebSrcRoot/sView.rpm.spectmp*

# backup unmodified sources into archive
pushd temp
tar -czvf sview-${aVersion}-${aRelease}.tar.gz sview-${aVersion}-${aRelease}
mkdir -p src
mv -f sview-${aVersion}-${aRelease}.tar.gz ./src
popd

# remove temporary files
rm -f temp/*.build
rm -f temp/*.buildinfo
rm -f temp/*.changes
rm -f temp/*.dsc
rm -f temp/*.tar.gz
rm -f temp/*.tar.xz

# build Debian package(s)
pushd ${aDebSrcRoot}
for aDistr in "${aDistribs[@]}"
do
  echo "Prepare source package for '$aDistr'"
  sed "s/unknown_distrib/${aDistr}/g"                  "debian/changelog.tmpl" > "debian/changelogtmp1"
  sed "s/unknown_version/1:${aDebVersion}~${aDistr}/g" "debian/changelogtmp1"  > "debian/changelogtmp2"
  sed "s/unknown_date/${aCurrentDate}/g"               "debian/changelogtmp2"  > "debian/changelog"
  rm -f debian/changelogtmp*

  mkdir -p ../$aDistr
  aLogFile="../$aDistr/build.log"
  rm -f $aLogFile
  debuild ${debuildParams[@]} 2>&1 | tee -a "$aLogFile"
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi

  aWarns=`grep "^E: " $aLogFile`
  while IFS= read -r aWarnIter ; do echo "::warning title=debuild::$aWarnIter"; done <<< "$aWarns"

  aWarns=`grep "^W: " $aLogFile`
  while IFS= read -r aWarnIter ; do echo "::warning title=debuild::$aWarnIter"; done <<< "$aWarns"

  mv -f ../*.build     ../$aDistr
  mv -f ../*.buildinfo ../$aDistr
  mv -f ../*.changes   ../$aDistr
  mv -f ../*.dsc       ../$aDistr
  mv -f ../*.tar.*z    ../$aDistr
  #mv -f ../*.tar.gz    ../$aDistr
  #mv -f ../*.tar.xz    ../$aDistr
  #mv -f ../*.deb       ../$aDistr
done

#for aDistr in "${aDistribs[@]}"
#do
#  dput ppa:sview/stable ../$aDistr/sview_${aDebVersion}~${aDistr}_source.changes
#done

# turn back to original directory
popd
