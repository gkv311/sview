#!/bin/bash

anImg=$1
aDelay=$2
anApp=$3

"$anApp" $4 $5 $6 $7 $8 $9 &
anAppPid=$!

rm -f "$anImg"
scrot -d $aDelay -F "$anImg"

anImgSize=$(wc -c <"$anImg")
if [ $anImgSize -lt 2000 ]; then
  # retry one more time
  mv -f "$anImg" "back-first-${anImg}"
  scrot -d 1 -F "$anImg"
fi

kill $anAppPid
