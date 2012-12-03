/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StMonitor_h_
#define __StMonitor_h_

#include <StStrings/StString.h>
#include <StTemplates/StRect.h>
#include <StCore/StEDIDParser.h>

typedef struct tagStMonitor {
    stUtf8_t      pnpid[8];  // model PnP identificator
    stUtf8_t    name[1024];  // model name
    stUtf8_t gpuName[1024];
    stUByte_t    edid[256];  // EDID data
    StRectI_t        vRect;  // virtual space (rectangle)
    int           systemId;  // monitor id in system, 0 is primary display
    int           freqCurr;  // frequency in Herz
    int            freqMax;  // maximum frequency in Herz
} StMonitor_t;

/**
 * Class represents monitor, connected to videocard.
 */
class ST_LOCAL StMonitor {

        private:

    StString    myPnpId; //!< PnPId identificator assigned by Microsoft
    StString     myName; //!< human-readable name for this display
    StString  myGpuName; //!< attached to this GPU
    StEDIDParser myEdid; //!< EDID data block if available
    StRectI_t    myRect; //!< virtual space (rectangle)
    int         mySysId; //!< monitor id
    int          myFreq; //!< frequency in Herz
    int       myFreqMax; //!< maximum frequency in Herz

        public:

    /**
     * Empty constructor.
     */
    StMonitor();

    /**
     * Copy constructor.
     */
    StMonitor(const StMonitor& theCopy);

    /**
     * Copy constructor (from structure).
     */
    StMonitor(const StMonitor_t& theMonStruct);

    /**
     * Create structure
     */
    StMonitor_t getStruct();

    bool isValid() const;

    int getId() const {
        return mySysId;
    }

    void setId(const int theMonId) {
        mySysId = theMonId;
    }

    const StString& getPnPId() const {
        return myPnpId;
    }

    void setPnPId(const StString& thePnpId) {
        myPnpId = thePnpId;
    }

    const StString& getName() const {
        return myName;
    }

    void setName(const StString& theName) {
        myName = theName;
    }

    void setVRect(const StRectI_t& theRect) {
        myRect = theRect;
    }

    const StRectI_t& getVRect() const {
        return myRect;
    }

    StRectI_t& changeVRect() {
        return myRect;
    }

    int getFreq() const {
        return myFreq;
    }

    void setFreq(const int theFrequency) {
        myFreq = theFrequency;
    }

    int getFreqMax() const {
        return myFreqMax;
    }

    void setFreqMax(const int theFrequencyMax) {
        myFreqMax = theFrequencyMax;
    }

    const StString& getGpuName() const {
        return myGpuName;
    }

    void setGpuName(const StString& theGpuName) {
        myGpuName = theGpuName;
    }

    const StEDIDParser& getEdid() const {
         return myEdid;
    }

    StEDIDParser& changeEdid() {
         return myEdid;
    }

    StString toString() const;

    bool operator==(const StMonitor& compare) const {
        if(&compare == this) {
            return true;
        }
        return compare.mySysId == mySysId
            && compare.myPnpId == myPnpId
            && compare.myRect  == myRect;
    }

    bool operator!=(const StMonitor& compare) const {
        return !this->operator==(compare);
    }

    bool operator>(const StMonitor& compare) const {
        return mySysId > compare.mySysId;
    }

    bool operator<(const StMonitor& compare) const {
        return mySysId < compare.mySysId;
    }

    bool operator>=(const StMonitor& compare) const {
        return mySysId >= compare.mySysId;
    }

    bool operator<=(const StMonitor& compare) const {
        return mySysId <= compare.mySysId;
    }

};

#endif //__StMonitor_h_
