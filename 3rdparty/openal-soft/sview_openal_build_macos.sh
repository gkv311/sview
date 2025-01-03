#!/bin/bash

# Auxiliary script for semi-automated building building of OpenAL Soft for macOS platform.
# Script should be placed into root of OpenAL Soft repository, edited with paths to CMake.
# https://github.com/kcat/openal-soft

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}
if [ -d "$aScriptPath" ]; then cd "$aScriptPath"; fi
aScriptPath="$PWD"
aProjName=${PWD##*/}

aSrcRoot=$aScriptPath
aNbJobs="$(getconf _NPROCESSORS_ONLN)"

PATH=/Applications/CMake.app/Contents/bin:$PATH

# build stages to perform
toCMake=1
toClean=1
toMake=1
toInstall=1
toPack=1

# OpenAL Soft 1.21 fails to build with targets below 10.15
#export MACOSX_DEPLOYMENT_TARGET=10.10
export MACOSX_DEPLOYMENT_TARGET=10.15
anAbi=arm64
#anAbi=x86_64
aPlatformAndCompiler=macos-$anAbi
aWorkDir=build/$aProjName-${aPlatformAndCompiler}-make
aDestDir=$aSrcRoot/build/$aProjName-$aPlatformAndCompiler
aLogFile=$aSrcRoot/build/build-${aPlatformAndCompiler}.log

mkdir -p $aWorkDir
rm    -f $aLogFile

aTimeZERO=$SECONDS
set -o pipefail

# (re)generate Make files
if [[ $toCMake == 1 ]]; then
  echo Configuring project for macOS...
  cmake -G "Unix Makefiles" \
 -D CMAKE_BUILD_TYPE:STRING="Release" \
 -D CMAKE_OSX_ARCHITECTURES:STRING="$anAbi" \
 -D CMAKE_INSTALL_PREFIX:PATH="$aDestDir" \
 -D CMAKE_INSTALL_NAME_DIR:STRING="@executable_path/../Frameworks" \
 -D ALSOFT_BACKEND_COREAUDIO:BOOL="ON" \
 -D ALSOFT_REQUIRE_COREAUDIO:BOOL="ON" \
 -D ALSOFT_BACKEND_WAVE:BOOL="OFF" \
 -D ALSOFT_EXAMPLES:BOOL="OFF" \
 -D ALSOFT_TESTS:BOOL="OFF" \
 -D ALSOFT_UTILS:BOOL="ON" \
 -D ALSOFT_NO_CONFIG_UTIL:BOOL="ON" \
 -S "$aSrcRoot" -B "$aWorkDir" 2>&1 | tee -a $aLogFile
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
fi

# clean up from previous build
if [[ $toClean == 1 ]]; then
  cmake --build "$aWorkDir" --config Release --target clean
fi

# build the project
if [[ $toMake == 1 ]]; then
  echo Building...
  cmake --build "$aWorkDir" --config Release -- -j $aNbJobs 2>&1 | tee -a $aLogFile
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
fi

# install the project
if [[ $toInstall == 1 ]]; then
  echo Installing into $aSrcRoot/build/$aPlatformAndCompiler...
  cmake --build "$aWorkDir" --config Release --target install 2>&1 | tee -a $aLogFile
  xcodebuild -version > $aDestDir/config.log
  clang --version >> $aDestDir/config.log
  echo MACOSX_DEPLOYMENT_TARGET=$MACOSX_DEPLOYMENT_TARGET>> $aDestDir/config.log
  cp -f $aSrcRoot/ChangeLog $aDestDir/ChangeLog
  cp -f $aSrcRoot/COPYING $aDestDir/COPYING
fi

# create an archive
if [[ $toPack == 1 ]]; then
  anArchName=$aProjName-$aPlatformAndCompiler.tar.bz2
  echo Creating an archive $aSrcRoot/build/$anArchName...
  rm $aSrcRoot/build/$anArchName &>/dev/null
  pushd $aDestDir
  tar -jcf $aSrcRoot/build/$anArchName *
  popd
fi

# finished
DURATION=$(($SECONDS - $aTimeZERO))
echo Total time: $DURATION sec
