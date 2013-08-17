/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StCore/StMonitor.h>

StMonitor::StMonitor()
: mySysId(0),
  myFreq(0),
  myFreqMax(0),
  myScale(1.0f) {
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
  myScale(theCopy.myScale) {
    //
}

bool StMonitor::isValid() const {
    return (myRect.width() > 1) && (myRect.height() > 1);
}

StString StMonitor::toString() const {
    StString aStereoType = myEdid.isValid() ? (StString(", stereo type: ") + myEdid.getStereoString()) : StString();
    return (StString()
        + "Monitor #" + mySysId + ", PnP ID: " + myPnpId + " (" + myName + ')'
        + aStereoType + '\n'
        + "Connected to " + myGpuName + '\n'
        + "freq= " + myFreq + "Hz / freqMax= " + myFreqMax + "Hz / scale= " + myScale + "\n"
        + myRect.toString()
    );
}
