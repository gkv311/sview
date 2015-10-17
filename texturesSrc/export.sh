#!/bin/bash
# Auxiliary script to export SVG images into grayscale PNG image.

anSvgName=$1
aPngName=$2
if [ "$aPngName" == "" ]; then aPngName=$1; fi
if [ ! -f "$anSvgName" ]; then anSvgName=${anSvgName}.svg; fi

aSizeList=(16 24 32 48 64 72 96 128 144 192 256)
for aSize in "${aSizeList[@]}"
do
  rm -f "${aPngName}${aSize}.png"

  inkscape --file "$anSvgName" --export-area-page --export-height ${aSize} --export-png "${aPngName}${aSize}.tmp.png"
  ffmpeg -i "${aPngName}${aSize}.tmp.png" -pix_fmt gray "${aPngName}${aSize}.png"
  rm -f "${aPngName}${aSize}.tmp.png"
done
