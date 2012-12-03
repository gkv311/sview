/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StCore/StMonitor.h>

StMonitor::StMonitor()
: myPnpId(),
  myName(),
  myGpuName(),
  myEdid(),
  myRect(),
  mySysId(0),
  myFreq(0),
  myFreqMax(0) {
    //
}

StMonitor::StMonitor(const StMonitor& theCopy)
: myPnpId(theCopy.myPnpId),
  myName(theCopy.myName),
  myGpuName(theCopy.myGpuName),
  myEdid(theCopy.myEdid),
  myRect(theCopy.myRect),
  mySysId(theCopy.mySysId),
  myFreq(theCopy.myFreq),
  myFreqMax(theCopy.myFreqMax) {
    //
}

StMonitor::StMonitor(const StMonitor_t& theMonStruct)
: myPnpId(theMonStruct.pnpid),
  myName(theMonStruct.name),
  myGpuName(theMonStruct.gpuName),
  myEdid(theMonStruct.edid),
  myRect(theMonStruct.vRect),
  mySysId(theMonStruct.systemId),
  myFreq(theMonStruct.freqCurr),
  myFreqMax(theMonStruct.freqMax) {
    //
}

bool StMonitor::isValid() const {
    return (myRect.width() > 1) && (myRect.height() > 1);
}

StMonitor_t StMonitor::getStruct() {
    StMonitor_t aMonStruct;
    stMemSet(&aMonStruct, 0, sizeof(StMonitor_t)); // this also automatically NULL-terminate all fixed-length strings!
    size_t aCopySize = stMin(size_t(7), myPnpId.getSize());
    stMemCpy(aMonStruct.pnpid, myPnpId.toCString(), aCopySize);
    aCopySize = stMin(size_t(1023), myName.getSize());
    stMemCpy(aMonStruct.name, myName.toCString(), aCopySize);
    aCopySize = stMin(size_t(1023), myGpuName.getSize());
    stMemCpy(aMonStruct.gpuName, myGpuName.toCString(), aCopySize);
    if(myEdid.getData() != NULL) {
        stMemCpy(aMonStruct.edid, myEdid.getData(), 128);
    }
    aMonStruct.vRect    = myRect;
    aMonStruct.systemId = mySysId;
    aMonStruct.freqCurr = myFreq;
    aMonStruct.freqMax  = myFreqMax;
    return aMonStruct;
}

StString StMonitor::toString() const {
    StString aStereoType = myEdid.isValid() ? (StString(", stereo type: ") + myEdid.getStereoString()) : StString();
    return (StString()
        + "Monitor #" + mySysId + ", PnP ID: " + myPnpId + " (" + myName + ')'
        + aStereoType + '\n'
        + "Connected to " + myGpuName + '\n'
        + "freq= " + myFreq + "Hz / freqMax= " + myFreqMax + "Hz\n"
        + myRect.toString()
    );
}
