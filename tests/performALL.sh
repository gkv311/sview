#!/usr/bash

export StCore32=$PWD/../bin/LINUX_gcc_DEBUG
export StCore64=$StCore32
export StCore=$StCore64

# Loop test
rm -f loopTestCMD.log
echo "Date: $(date)" > testLoopCMD.log
bash testLoop.sh        2>&1 | tee -a testLoopCMD.log
$(date) > testOutputsCMD.log
bash testOutputs.sh     2>&1 | tee -a testOutputsCMD.log
$(date) > testOutputsCMD.log
bash testImageViewer.sh 2>&1 | tee -a testOutputsCMD.log
