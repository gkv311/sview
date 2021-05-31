/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StMonitor_h_
#define __StMonitor_h_

#include <StStrings/StString.h>
#include <StTemplates/StRect.h>
#include <StCore/StEDIDParser.h>

/**
 * Class represents monitor connected to videocard.
 */
class StMonitor {

        public:

    /**
     * Monitor orientation.
     */
    enum Orientation {
        Orientation_Landscape, //!< landscape orientation (default)
        Orientation_Portrait,  //!< portrait  orientation
    };

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StMonitor();

    /**
     * Copy constructor.
     */
    ST_CPPEXPORT StMonitor(const StMonitor& theCopy);

    ST_CPPEXPORT bool isValid() const;

    ST_LOCAL int getId() const {
        return mySysId;
    }

    ST_LOCAL void setId(const int theMonId) {
        mySysId = theMonId;
    }

    /**
     * @return model PnP identifier assigned by Microsoft
     */
    ST_LOCAL const StString& getPnPId() const {
        return myPnpId;
    }

    /**
     * @param thePnpId model PnP identifier
     * Notice that this method do not affect EDID data.
     */
    ST_LOCAL void setPnPId(const StString& thePnpId) {
        myPnpId = thePnpId;
    }

    /**
     * @return model name
     */
    ST_LOCAL const StString& getName() const {
        return myName;
    }

    /**
     * @param theName model name
     */
    ST_LOCAL void setName(const StString& theName) {
        myName = theName;
    }

    /**
     * @return virtual space (rectangle), how this monitor is arranged with another displays
     */
    ST_LOCAL const StRectI_t& getVRect() const {
        return myRect;
    }

    /**
     * @return virtual space (rectangle)
     */
    ST_LOCAL StRectI_t& changeVRect() {
        return myRect;
    }

    /**
     * @param theRect virtual space (rectangle)
     */
    ST_LOCAL void setVRect(const StRectI_t& theRect) {
        myRect = theRect;
    }

    /**
     * @return pixels density scale factor
     */
    ST_LOCAL float getScale() const {
        return myScale;
    }

    /**
     * @param theScale pixels density scale factor
     */
    ST_LOCAL void setScale(const float theScale) {
        myScale = theScale;
    }

    /**
     * @return current vertical refresh rate
     */
    ST_LOCAL float getFreq() const { return myFreq; }

    /**
     * @param theFrequency current vertical refresh rate
     */
    ST_LOCAL void setFreq(float theFrequency) { myFreq = theFrequency; }

    /**
     * @return maximal vertical refresh rate
     */
    ST_LOCAL float getFreqMax() const { return myFreqMax; }

    /**
     * @param theFrequencyMax maximal vertical refresh rate
     */
    ST_LOCAL void setFreqMax(float theFrequencyMax) { myFreqMax = theFrequencyMax; }

    /**
     * @return GPU to which monitor is connected to
     */
    ST_LOCAL const StString& getGpuName() const {
        return myGpuName;
    }

    /**
     * @param theGpuName GPU to which monitor is connected to
     */
    ST_LOCAL void setGpuName(const StString& theGpuName) {
        myGpuName = theGpuName;
    }

    /**
     * @return associated EDID data
     */
    ST_LOCAL const StEDIDParser& getEdid() const {
         return myEdid;
    }

    /**
     * @return associated EDID data
     */
    ST_LOCAL StEDIDParser& changeEdid() {
         return myEdid;
    }

    /**
     * Return monitor orientation.
     */
    ST_LOCAL StMonitor::Orientation getOrientation() const {
        return myOrient;
    }

    /**
     * Setup monitor orientation.
     */
    ST_LOCAL void setOrientation(const StMonitor::Orientation theOrientation) {
        myOrient = theOrientation;
    }

    /**
     * @return human-readable string with monitor description
     */
    ST_CPPEXPORT StString toString() const;

    ST_LOCAL bool operator==(const StMonitor& compare) const {
        if(&compare == this) {
            return true;
        }
        return compare.mySysId == mySysId
            && compare.myPnpId == myPnpId
            && compare.myRect  == myRect;
    }

    ST_LOCAL bool operator!=(const StMonitor& compare) const {
        return !this->operator==(compare);
    }

    ST_LOCAL bool operator>(const StMonitor& compare) const {
        return mySysId > compare.mySysId;
    }

    ST_LOCAL bool operator<(const StMonitor& compare) const {
        return mySysId < compare.mySysId;
    }

    ST_LOCAL bool operator>=(const StMonitor& compare) const {
        return mySysId >= compare.mySysId;
    }

    ST_LOCAL bool operator<=(const StMonitor& compare) const {
        return mySysId <= compare.mySysId;
    }

        private:

    StString     myPnpId;   //!< PnPId identifier assigned by Microsoft
    StString     myName;    //!< human-readable name for this display
    StString     myGpuName; //!< attached to this GPU
    StEDIDParser myEdid;    //!< EDID data block if available
    StRectI_t    myRect;    //!< virtual space (rectangle)
    int          mySysId;   //!< monitor id
    float        myFreq;    //!< frequency in Hertz
    float        myFreqMax; //!< maximum frequency in Hertz
    float        myScale;   //!< hight pixel density scale factor
    Orientation  myOrient;  //!< monitor orientation

};

#endif // __StMonitor_h_
