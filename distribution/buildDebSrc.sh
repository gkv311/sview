#!/bin/bash

# This script generates the DEB source package.
#
# Copyright © Kirill Gavrilov, 2012-2013

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}
if [ -d "$aScriptPath" ]; then
    cd "$aScriptPath"
fi

YEAR=$(date +"%y")
MONTH=$(date +"%m")
DAY=$(date +"%d")
aVersion=${YEAR}.${MONTH}
aRelease=1
aDebVersion=${aVersion}-${aRelease}
aDebSrcRoot=temp/sview-${aDebVersion}

# copy common files
rm --force --recursive     $aDebSrcRoot
mkdir -p $aDebSrcRoot
cp -f -r ../debian         $aDebSrcRoot/
cp -f -r ../share          $aDebSrcRoot/
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
cp -f    ../Makefile       $aDebSrcRoot/
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

pushd .
cd temp
tar -czvf sview-${aVersion}-${aRelease}.tar.gz sview-${aVersion}-${aRelease}
mkdir -p src
mv -f sview-${aVersion}-${aRelease}.tar.gz ./src
popd
