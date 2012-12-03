#!/usr/bash

# This is test script launches sView several times in loop (stable test).
# $StCore environment variable should be set before to the sView path.

rm -f testLoop.log
aLoopLimit=10;
aLoopIter=1;
while [ $aLoopIter -le $aLoopLimit ]; do
  echo " -=- Loop TEST $aLoopIter/$aLoopLimit -=- " | tee -a testLoop.log
  bash timeout.sh -t 5 "$StCore/sView" --out=StOutAnaglyph --viewMode=flat --srcFormat=crossEyed - "$StCore/demo.jps" &>> testLoop.log
  aLoopIter=`expr $aLoopIter + 1`;
done
echo " -=- Loop TEST finished -=- " >> testLoop.log
