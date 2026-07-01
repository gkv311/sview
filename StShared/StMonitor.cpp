/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StCore/StMonitor.h>

#include <sstream>

StMonitor::StMonitor() {
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
  myOrient(theCopy.myOrient),
  myIsWideGamut(theCopy.myIsWideGamut) {
    //
}

StMonitor& StMonitor::operator=(const StMonitor& theCopy) {
    myPnpId = theCopy.myPnpId;
    myName = theCopy.myName;
    myGpuName = theCopy.myGpuName;
    myEdid = theCopy.myEdid;
    myRect = theCopy.myRect;
    mySysId = theCopy.mySysId;
    myFreq = theCopy.myFreq;
    myFreqMax = theCopy.myFreqMax;
    myScale = theCopy.myScale;
    myOrient = theCopy.myOrient;
    myIsWideGamut = theCopy.myIsWideGamut;
    return *this;
}

bool StMonitor::isValid() const {
    return (myRect.width() > 1) && (myRect.height() > 1);
}

StString StMonitor::toString() const {
    std::stringstream aStr;
    aStr << "Monitor #" << mySysId;

    if (!myPnpId.isEmpty())
        aStr << ", PnP ID: " << myPnpId;

    aStr << ", scale: " << myScale;

    if (myEdid.isValid())
        aStr << ", stereo type: " << myEdid.getStereoString();

    if (myIsWideGamut)
        aStr << ", wide gamut";

    if (myFreq > 0)
        aStr << " | freq: " << myFreq << "Hz";

    if (myFreqMax > 0)
        aStr << " (max: " << myFreqMax << "Hz)";

    if (!myName.isEmpty())
        aStr << ", name: '" << myName << "'";

    if (myOrient == Orientation_Portrait)
        aStr << ", portrait";

    if (!myGpuName.isEmpty())
        aStr << "\nConnected to " << myGpuName;

    aStr << "\n" << myRect.toString();
    return aStr.str().c_str();
}
