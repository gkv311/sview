#!/bin/bash

# This script generates the DMG package.
#
# Copyright © Kirill Gavrilov, 2011-2016

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}
if [ -d "$aScriptPath" ]; then
    cd "$aScriptPath"
fi

aSystem=`uname -s`
buildRoot=temp/sView-$aSystem
aTmpDmg=temp/pack.temp.dmg

aVerType="_";
for i in $*
do
  if [ "$i" == "ST_RELEASE" ]; then
    aVerType="_";
  elif [ "$i" == "ST_ALPHA" ]; then
    aVerType="alpha"
  elif [ "$i" == "ST_RELEASE_CANDIDATE" ]; then
    aVerType="rc"
  fi
done

# prepare directories
rm -f -r $buildRoot
mkdir -p $buildRoot/.background
#mkdir -p $buildRoot/sView.app/Contents/Resources/info/
mkdir -p $buildRoot/sView.app/Contents/MacOS/info/

# copy common files
cp -f media/sView_SetupMac.png $buildRoot/.background/
cp -f info/changelog.txt     $buildRoot/sView.app/Contents/MacOS/info/
cp -f ../license-gpl-3.0.txt $buildRoot/sView.app/Contents/MacOS/info/license.txt

# copy sView compiled files
cp -f -R ../build/sView.app/* $buildRoot/sView.app/

# create symlink to Applications
ln -f -s /Applications "$buildRoot/"

aYear=$(date +"%y")
aMonth=$(date +"%m")
aDay=$(date +"%d")
aDmgVersion=${aYear}.${aMonth}${aVerType}${aDay}

# update version
sed -e s/"<string>1.0"/"<string>${aYear}.${aMonth}"/g "$buildRoot/sView.app/Contents/Info.plist" > "$buildRoot/sView.app/Contents/Info.plist.copy"
mv -f "$buildRoot/sView.app/Contents/Info.plist.copy" "$buildRoot/sView.app/Contents/Info.plist"

aTitle="sView"
aBackgroundPictureName="sView_SetupMac.png"
aFinalDMGName="sViewSetup_v.${aDmgVersion}.dmg"

# Create a R/W DMG; it must be larger than the result will be
rm -f ${aTmpDmg}
hdiutil create -srcfolder "${buildRoot}" -volname "${aTitle}" -fs HFS+ \
               -fsargs "-c c=64,a=16,e=16" -format UDRW -size 50000k ${aTmpDmg}

# Mount the disk image, and store the device name
# (we may need to use sleep for a few seconds after this operation)
aDevice=$(hdiutil attach -readwrite -noverify -noautoopen "${aTmpDmg}" | \
         egrep '^/dev/' | sed 1q | awk '{print $1}')
sleep 5

# Use AppleScript to set the visual styles
echo '
    tell application "Finder"
        tell disk "'${aTitle}'"
            open
            set current view of container window to icon view
            set toolbar   visible of container window to false
            set statusbar visible of container window to false
            set the bounds of container window to {400, 100, 1020, 400}
            set theViewOptions to the icon view options of container window
            set arrangement of theViewOptions to not arranged
            set icon size of theViewOptions to 96
            set background picture of theViewOptions to file ".background:'${aBackgroundPictureName}'"
            set position of item "'sView'"      of container window to {180, 140}
            set position of item "Applications" of container window to {460, 140}
            close
            open
            update without registering applications
            delay 5
            eject
        end tell
    end tell
' | osascript

hdiutil detach ${aDevice}
aDevice=$(hdiutil attach -readwrite -noverify -noautoopen "${aTmpDmg}" | \
         egrep '^/dev/' | sed 1q | awk '{print $1}')
sleep 5

# finalize the DMG by setting permissions properly, compressing and releasing it
chmod -Rf go-w /Volumes/"${aTitle}"
sync
sync
hdiutil detach ${aDevice}
hdiutil convert "${aTmpDmg}" -format UDZO -imagekey zlib-level=9 -o "${aFinalDMGName}"
rm -f ${aTmpDmg}

# prepare directories and move the packages
mkdir -p repository/mac
mv -f "${aFinalDMGName}" repository/mac/
