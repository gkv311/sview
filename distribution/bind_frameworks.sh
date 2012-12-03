#!/bin/bash

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}
if [ -d "$aScriptPath" ]; then cd "$aScriptPath"; fi
aScriptPath="$PWD"

aRoot="$aScriptPath"
aFrameworksRoot="@executable_path/../Frameworks"

replaceDepLibs() {
    aTarget="$1"
    for aLib in "libavutil.dylib" "libavformat.dylib" "libavcodec.dylib" "libswscale.dylib"
    do
        install_name_tool -change /usr/local/lib/$aLib ${aFrameworksRoot}/${aLib} "$aTarget"
    done
    otool -L "$aTarget"
}

replaceSelfPath() {
    aTarget="$1"
    aFileName=`basename "$aTarget"`;
    install_name_tool -id ${aFrameworksRoot}/${aFileName} "${aTarget}"
}

aFilesList=$aScriptPath/*.dylib
for aFile in $aFilesList; do
    if [ -f "$aFile" ] && [ ! -L "$aFile" ] ; then
        replaceSelfPath "$aFile"
        replaceDepLibs  "$aFile"
        otool -L "$aFile"
    fi
done

#replaceDepLibs "${aRoot}/StDrawers/StImageViewer.dylib"

#echo "aScriptPath= '$aScriptPath'"
#pause
