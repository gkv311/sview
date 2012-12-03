#!/bin/bash

# This script perform compilation of sView.
#
# Copyright © Kirill Gavrilov, 2010-2012

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}; if [ -d "${aScriptPath}" ]; then cd "$aScriptPath"; fi; aScriptPath="$PWD";

releaseStatus="RELEASE"

for i in $*
do
  if [ "$i" == "RELEASE" ]; then
    releaseStatus="RELEASE";
  elif [ "$i" == "ALPHA" ]; then
    releaseStatus="ALPHA"
  elif [ "$i" == "RELEASE_CANDIDATE" ]; then
    releaseStatus="RELEASE_CANDIDATE"
  fi
done

# detect architecture
aSystem=`uname -s`
anArch=`uname -m`
if [ "$anArch" != "x86_64" ] && [ "$anArch" != "ia64" ]; then
    anArch=i386
else
    anArch=amd64
fi

buildRoot=temp/sView-$aSystem-$anArch

# make backup of default config file
mv -f ../include/stconfig.conf ../include/stconfig.conf.buildbak

SVIEW_BUILD_CONF=../include/stconfig.conf

echo Build configuration:
echo     Target="$aSystem gcc $anArch"

# START creating config file
if [ "$releaseStatus" == "RELEASE" ]; then
  echo     Timebomb=OFF
  aTimeBombDefine=""
else
  echo     Timebomb=ON
  aTimeBombDefine=$(cat << EOF
#ifndef __ST_TIMEBOMB__
    #define __ST_TIMEBOMB__
#endif
EOF
)
fi

cat > $SVIEW_BUILD_CONF << EOF
#ifndef __stConfig_conf_
#define __stConfig_conf_

$aTimeBombDefine

#ifndef SVIEW_SDK_VER_STATUS
    #define SVIEW_SDK_VER_STATUS $releaseStatus
#endif

#ifndef SVIEW_SDK_VERSION_AUTO
    #define SVIEW_SDK_VERSION_AUTO
#endif

#endif //__stConfig_conf_

EOF
# END creating config file

aConfig="LINUX_gcc"
aCodeBlocksCmd="codeblocks"
if [ "$aSystem" == "Darwin" ]; then
    aConfig="MAC_gcc"
    aCodeBlocksCmd="/Applications/CodeBlocks.app/Contents/MacOS/CodeBlocks"
fi

mkdir -p "${HOME}/.codeblocks"
cp -f "${aScriptPath}/codeblocks/min_gcc.conf" "${HOME}/.codeblocks/"

mkdir -p "${aScriptPath}/../bin/$aConfig"

# start virtual X-server for C::B GUI
hasXvfb=`which Xvfb`
if [ -x "${hasXvfb}" ]; then
    Xvfb :20 -screen 0 640x480x24 &
    aXvfbPid=$!
    anOldDisplay=$DISPLAY
    export DISPLAY=:20
fi;

# perform sView rebuild
$aCodeBlocksCmd --no-splash-screen --profile="min_gcc" --rebuild --target="$aConfig" \
                ../workspace.workspace \
                &>"${aScriptPath}/../bin/$aConfig/build.log"

if [ -x "${hasXvfb}" ]; then
    kill $aXvfbPid
    export DISPLAY=$anOldDisplay
fi;

# move default config file back
mv -f ../include/stconfig.conf.buildbak ../include/stconfig.conf
