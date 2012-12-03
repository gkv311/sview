#!/usr/bash

# This is test script alternately loads sView with all StRenderers.
# $StCore environment variable should be set before to the sView path.

rm -f testOutputs.log
stRenderersLibs=$StCore/StRenderers/*.so
for aRenderer in $stRenderersLibs
do
  aRendererShort="${aRenderer##*/}"
  echo " -=- StRenderers TEST. $aRendererShort -=- " | tee -a testOutputs.log
  bash timeout.sh -t 5 "$StCore/sView" --out="$aRenderer" --viewMode=flat --srcFormat=crossEyed - "$StCore/demo.jps" &>> testOutputs.log
done

echo " -=- StRenderers TEST. Auto -=- " | tee -a testOutputs.log
bash timeout.sh -t 5 "$StCore/sView" --out=Auto --viewMode=flat --srcFormat=crossEyed - "$StCore/demo.jps" &>> testOutputs.log
echo " -=- StRenderers TEST finished -=- " >> testOutputs.log
