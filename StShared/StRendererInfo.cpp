/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StCore/StRendererInfo.h>
#include <StCore/StRendererPlugin.h>
#include <StFile/StFileNode.h>

StRendererInfo::StRendererInfo()
: myIsValid(false) {
    //
}

StRendererInfo::StRendererInfo(const StRendererInfo& theCopy)
: myRendererPath(theCopy.myRendererPath),
  myAboutString(theCopy.myAboutString),
  myDeviceList(theCopy.myDeviceList),
  myIsValid(theCopy.myIsValid) {
    //
}

StRendererInfo::StRendererInfo(const StRendererInfo_t& theCopy)
: myRendererPath(theCopy.rendererPath),
  myAboutString(theCopy.aboutString),
  myDeviceList(theCopy),
  myIsValid(true) {
    //
}

StRendererInfo::StRendererInfo(const StString& theRendererPath,
                               const bool      theToDetectPriority)
: myRendererPath(theRendererPath),
  myIsValid(false) {
    StRendererPlugin aRenderer;
    if(!aRenderer.InitLibrary(myRendererPath)) {
        return;
    }

    const StRendererInfo_t* aDevInfo = aRenderer.GetDevicesInfo(theToDetectPriority);
    if(aDevInfo == NULL || aDevInfo->count == 0) {
        return;
    }

    myDeviceList  = StStereoDeviceInfoList(*aDevInfo);
    myAboutString = aDevInfo->aboutString;
    myIsValid     = true;
}

const StRendererInfo& StRendererInfo::operator=(const StRendererInfo& theCopy) {
    if(this != &theCopy) {
        myRendererPath = theCopy.myRendererPath;
        myAboutString  = theCopy.myAboutString;
        myDeviceList   = theCopy.myDeviceList;
    }
    return (*this);
}

StString StRendererInfo::getTitle() const {
    StString aDummy, aFile, aTitle;
    StFileNode::getFolderAndFile(myRendererPath, aDummy, aFile);
    StFileNode::getNameAndExtension(aFile, aTitle, aDummy);
    return aTitle;
}

int StRendererInfo::getSupportLevel() const {
    int maxSupportLevel = ST_DEVICE_SUPPORT_NONE;
    for(size_t devId = 0; devId < myDeviceList.size(); devId++) {
        int supportLevel = myDeviceList[devId].getDetectionLevel();
        maxSupportLevel = supportLevel > maxSupportLevel ? supportLevel : maxSupportLevel;
    }
    return maxSupportLevel;
}

bool StRendererInfo::operator==(const StRendererInfo& theCompare) const {
    if(&theCompare == this) {
        return true;
    }
#if(defined(_WIN32) || defined(__WIN32__))
    return theCompare.myRendererPath.isEqualsIgnoreCase(myRendererPath);
#else
    return theCompare.myRendererPath == myRendererPath;
#endif
}

bool StRendererInfo::operator!=(const StRendererInfo& theCompare) const {
    if(&theCompare == this) {
        return false;
    }
#if(defined(_WIN32) || defined(__WIN32__))
    return !theCompare.myRendererPath.isEqualsIgnoreCase(myRendererPath);
#else
    return theCompare.myRendererPath != myRendererPath;
#endif
}

StString StRendererInfo::toString() const {
    return StString("Renderer Path = '") + myRendererPath + "'. Full Device list:\n" + myDeviceList.toString();
}
