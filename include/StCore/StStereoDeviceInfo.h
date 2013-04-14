/**
 * Copyright © 2009 Kirill Gavrilov <kirill@sview.ru>
 *
 * StCore library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StCore library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StStereoDeviceInfo_h_
#define __StStereoDeviceInfo_h_

#include "StStereoDeviceInfo_t.h"

#ifdef __cplusplus

#include <StStrings/StString.h>

class StStereoDeviceInfo {

        public:

    StStereoDeviceInfo()
    : myDetectionLevel(ST_DEVICE_SUPPORT_NONE) {
        //
    }

    StStereoDeviceInfo(const StString& theStringId,
                       const StString& theName,
                       const StString& theDescription,
                       const int       theDetectionLevel)
    : myStringId(theStringId),
      myName(theName),
      myDescription(theDescription),
      myDetectionLevel(theDetectionLevel) {
        //
    }

    StStereoDeviceInfo(const StStereoDeviceInfo& theCopy)
    : myStringId(theCopy.myStringId),
      myName(theCopy.myName),
      myDescription(theCopy.myDescription),
      myDetectionLevel(theCopy.myDetectionLevel) {
        //
    }

    StStereoDeviceInfo(const StStereoDeviceInfo_t& theCopy)
    : myStringId(StString(theCopy.stringId)),
      myName(StString(theCopy.name)),
      myDescription(StString(theCopy.description)),
      myDetectionLevel(theCopy.detectionLevel) {
        //
    }

    const StStereoDeviceInfo& operator=(const StStereoDeviceInfo& theCopy) {
        if(this != &theCopy) {
            myStringId       = theCopy.myStringId;
            myName           = theCopy.myName;
            myDescription    = theCopy.myDescription;
            myDetectionLevel = theCopy.myDetectionLevel;
        }
        return (*this);
    }

    const StString& getStringId() const {
        return myStringId;
    }

    void setStringId(const StString& theStringId) {
        myStringId = theStringId;
    }

    const StString& getName() const {
        return myName;
    }

    void setName(const StString& theName) {
        myName = theName;
    }

    const StString& getDescription() const {
        return myDescription;
    }

    void setDescription(const StString& theDescription) {
        myDescription = theDescription;
    }

    const int getDetectionLevel() const {
        return myDetectionLevel;
    }

    void setDetectionLevel(const int theDetectionLevel) {
        myDetectionLevel = theDetectionLevel;
    }

    bool operator==(const StStereoDeviceInfo& theCompare) const {
        if(&theCompare == this) {
            return true;
        }
        return theCompare.myStringId == myStringId;
    }

    bool operator!=(const StStereoDeviceInfo& theCompare) const {
        if(&theCompare == this) {
            return false;
        }
        return theCompare.myStringId != myStringId;
    }

    bool operator>(const StStereoDeviceInfo& theCompare) const {
        if(&theCompare == this) {
            return false;
        }
        return myDetectionLevel > theCompare.myDetectionLevel;
    }

    bool operator<(const StStereoDeviceInfo& theCompare) const {
        if(&theCompare == this) {
            return false;
        }
        return myDetectionLevel < theCompare.myDetectionLevel;
    }

    bool operator>=(const StStereoDeviceInfo& theCompare) const {
        if(&theCompare == this) {
            return true;
        }
        return myDetectionLevel >= theCompare.myDetectionLevel;
    }

    bool operator<=(const StStereoDeviceInfo& theCompare) const {
        if(&theCompare == this) {
            return true;
        }
        return myDetectionLevel <= theCompare.myDetectionLevel;
    }

    StString toString() const {
        return StString(myDetectionLevel) + ':' + myStringId + ':' + myName + ':' + myDescription;
    }

        private:

    StString myStringId;
    StString myName;
    StString myDescription;
    int      myDetectionLevel;

};

#endif //__cplusplus
#endif //__StStereoDeviceInfo_h_
