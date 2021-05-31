/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StCore/StMonitor.h>

StMonitor::StMonitor()
: mySysId(0),
  myFreq(0),
  myFreqMax(0),
  myScale(1.0f),
  myOrient(Orientation_Landscape) {
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
  myFreqMax(theCopy.myFreqMax),
  myScale(theCopy.myScale),
  myOrient(theCopy.myOrient) {
    //
}

bool StMonitor::isValid() const {
    return (myRect.width() > 1) && (myRect.height() > 1);
}

StString StMonitor::toString() const {
    StString aStereoType = myEdid.isValid() ? (StString(", stereo type: ") + myEdid.getStereoString()) : StString();
    return StString()
        + "Monitor #" + mySysId + ", PnP ID: " + myPnpId + " (" + myName + ")"
        + (myOrient == Orientation_Portrait ? ", portrait" : "") + aStereoType + '\n'
        + "Connected to " + myGpuName + '\n'
        + "freq= " + myFreq + "Hz / freqMax= " + myFreqMax + "Hz / scale= " + myScale + "\n"
        + myRect.toString();
}
