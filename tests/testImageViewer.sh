#!/usr/bash

# This is test script performs test on StImageViewer plugin.
# $StCore environment variable should be set before to the sView path.

rm -f testImageViewer.log

echo " -=- StImageViewer TEST. srcFormat=mono -=- " | tee -a testImageViewer.log
bash timeout.sh -t 5 "$StCore/sView" --in=StImageViewer --out=StOutAnaglyph --viewMode=flat --srcFormat=mono - "$StCore/demo.jps" &>> testImageViewer.log

echo " -=- StImageViewer TEST. srcFormat=crossEyed -=- " | tee -a testImageViewer.log
bash timeout.sh -t 5 "$StCore/sView" --in=StImageViewer --out=StOutAnaglyph --viewMode=flat --srcFormat=crossEyed - "$StCore/demo.jps" &>> testImageViewer.log

echo " -=- StImageViewer TEST. srcFormat=overUnderLR -=- " | tee -a testImageViewer.log
bash timeout.sh -t 5 "$StCore/sView" --in=StImageViewer --out=StOutAnaglyph --viewMode=flat --srcFormat=overUnderLR - "$StCore/demo.jps" &>> testImageViewer.log

echo " -=- StImageViewer TEST. srcFormat=interlaceRow -=- " | tee -a testImageViewer.log
bash timeout.sh -t 5 "$StCore/sView" --in=StImageViewer --out=StOutAnaglyph --viewMode=flat --srcFormat=interlaceRow - "$StCore/demo.jps" &>> testImageViewer.log

echo " -=- StImageViewer TEST. srcFormat=auto -=- " | tee -a testImageViewer.log
bash timeout.sh -t 5 "$StCore/sView" --in=StImageViewer --out=StOutAnaglyph --viewMode=flat --srcFormat=auto - "$StCore/demo.jps" &>> testImageViewer.log

echo " -=- StImageViewer TEST. viewMode=sphere -=- " | tee -a testImageViewer.log
bash timeout.sh -t 5 "$StCore/sView" --in=StImageViewer --out=StOutAnaglyph --viewMode=sphere --srcFormat=auto - "$StCore/demo.jps" &>> testImageViewer.log


echo " -=- StImageViewer TEST finished -=- " >> testImageViewer.log
