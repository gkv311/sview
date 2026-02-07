#!/bin/bash

# Script for building sView for Android platform.

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}; if [ -d "${aScriptPath}" ]; then cd "$aScriptPath"; fi; aScriptPath="$PWD";

aSrcRoot=$aScriptPath/..
aBuildPath=$aSrcRoot/build

PATH=$HOME/develop/tools/android-build-tools-26.0.3:$PATH

ANDROID_KEY="sview android key"
#ANDROID_KEYSTORE="$HOME/sview_debug.key"
#ANDROID_KEYSTORE_PASSWORD="sview_pswd"
#ANDROID_KEY_PASSWORD="sview_pswd"

ANDROID_KEYSTORE_REAL="$HOME/develop/gkv_key"
ANDROID_KEYSTORE="$aBuildPath/sview.key"
ANDROID_KEYSTORE_PASSWORD=`zenity --password --title="Android keystore"`
ANDROID_KEY_PASSWORD=`zenity --password --title="Android key"`
keytool -v -list -keystore "$ANDROID_KEYSTORE_REAL" -alias "$ANDROID_KEY" -storepass $ANDROID_KEYSTORE_PASSWORD
retVal=$?
if [ $retVal -ne 0 ]; then
  echo "Wrong password?"
  exit $retVal
fi

SVIEW_APK_UNSIGNED=$aBuildPath/sView-Release-tmp.apk
#SVIEW_APK_UNSIGNED=$aBuildPath/sView-Release.unsigned.apk.tmp
SVIEW_APK_SIGNED=$aBuildPath/sView-Release-tmp-signed.apk
SVIEW_APK_FINAL=$aBuildPath/sView-Release-final.apk

echo "Run jarsigner"
jarsigner -sigalg SHA1withRSA -digestalg SHA1 -keystore "${ANDROID_KEYSTORE}" -storepass "${ANDROID_KEYSTORE_PASSWORD}" -keypass "${ANDROID_KEY_PASSWORD}" \
          -verbose -signedjar "${SVIEW_APK_SIGNED}" "${SVIEW_APK_UNSIGNED}" "${ANDROID_KEY}"
if [ $retVal -ne 0 ]; then
  exit $retVal
fi

echo "Run zipalign"
zipalign -v -f 4 ${SVIEW_APK_SIGNED} ${SVIEW_APK_FINAL}
if [ $retVal -ne 0 ]; then
  exit $retVal
fi
