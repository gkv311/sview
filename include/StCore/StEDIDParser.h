/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StEDIDParser_h_
#define __StEDIDParser_h_

#include <StStrings/StString.h>

/**
 * EDID parser class, version 1.x.
 */
class ST_LOCAL StEDIDParser {

        public:

    typedef enum {
        STEREO_NO,                 //!< normal display - no stereo
        STEREO_PAGEFLIP_R,         //!< field-sequential stereo, right image when stereo sync signal = 1
        STEREO_PAGEFLIP_L,         //!< field-sequential stereo
        STEREO_INTERLEAVED_2WAY_R, //!< 2-way interleaved stereo, right image on even lines
        STEREO_INTERLEAVED_2WAY_L, //!< 2-way interleaved stereo, left  image on even lines
        STEREO_INTERLEAVED_4WAY,   //!< 4-way interleaved stereo
        STEREO_SIDEBYSIDE,         //!< SideBySide interleaved stereo
    } stEdid1Stereo_t;

        private:

    stUByte_t* myData;

        public:

    /**
     * Empty constructor.
     */
    StEDIDParser();

    /**
     * Initialize the parser.
     * @param theData (const stUByte_t* ) - the data should be 128 bytes long.
     */
    StEDIDParser(const stUByte_t* theData);

    /**
     * Copy constructor.
     */
    StEDIDParser(const StEDIDParser& theCopy);
    const StEDIDParser& operator=(const StEDIDParser& theCopy);

    /**
     * Remove current data.
     */
    void clear();

    ~StEDIDParser();

    const stUByte_t* getData() const {
        return myData;
    }

    /**
     * Initialize the parser.
     * @param theData (const stUByte_t* ) - the data should be 128 bytes long.
     */
    void init(const stUByte_t* theData);

    bool isFirstVersion() const;

    /**
     * Returns true if checksum is valid.
     */
    bool isValid() const;

    /**
     * Update checksum.
     */
    void validate();

        public:

    // retrieve information from EDID
    // you should check isValid() before!!!

    /**
     * Returns EDID version number.
     */
    unsigned int getVersion() const;

    /**
     * Returns EDID revision number.
     */
    unsigned int getRevision() const;

    /**
     * Year of Manufacture.
     */
    unsigned int getYear() const;

    /**
     * Week of Manufacture. This varies by manufacturer.
     * One way is to count January 1–7 as week 1, January 8–15 as week 2 and so on.
     * Some count based on the week number (Sunday-Saturday). Valid range is 1-54.
     */
    unsigned int getWeek() const;

    double getGamma() const;

    /**
     * Returns the model name.
     */
    StString getName() const;

    /**
     * Parse the data and extract PnPID.
     */
    StString getPnPId() const;

    void setPnPId(const StString& thePnPIdString);

    /**
     * Retrieve the stereo flag.
     */
    stEdid1Stereo_t getStereoFlag() const;
    StString getStereoString() const;

    bool operator==(const StEDIDParser& theCompare) const {
        if(&theCompare == this) {
            return true;
        }
        return (myData != NULL) && (theCompare.myData != NULL) && stAreEqual(myData, theCompare.myData, 128);
    }

    bool operator!=(const StEDIDParser& theCompare) const {
        return !operator==(theCompare);
    }

    bool operator>(const StEDIDParser& theCompare) const {
        if(&theCompare == this) {
            return false;
        }
        return getPnPId() > theCompare.getPnPId();
    }

    bool operator<(const StEDIDParser& theCompare) const {
        if(&theCompare == this) {
            return false;
        }
        return getPnPId() < theCompare.getPnPId();
    }

    bool operator>=(const StEDIDParser& theCompare) const {
        if(&theCompare == this) {
            return true;
        }
        return getPnPId() >= theCompare.getPnPId();
    }

    bool operator<=(const StEDIDParser& theCompare) const {
        if(&theCompare == this) {
            return true;
        }
        return getPnPId() <= theCompare.getPnPId();
    }

};

#endif //__StEDIDParser_h_
