/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2010-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StEDIDParser_h_
#define __StEDIDParser_h_

#include <StStrings/StString.h>

/**
 * EDID parser class, for EDID version 1.x.
 */
class StEDIDParser {

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

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StEDIDParser();

    /**
     * Initialize the parser.
     * @param theData pointer to EDID data
     * @param theSize size of data (should be at least 128 bytes long)
     */
    ST_CPPEXPORT StEDIDParser(const stUByte_t*   theData,
                              const unsigned int theSize = 128);

    /**
     * Copy constructor.
     */
    ST_CPPEXPORT StEDIDParser(const StEDIDParser& theCopy);

    /**
     * Copy operator.
     */
    ST_CPPEXPORT const StEDIDParser& operator=(const StEDIDParser& theCopy);

    /**
     * Remove current data.
     */
    ST_CPPEXPORT void clear();

    /**
     * Destructor
     */
    ST_CPPEXPORT ~StEDIDParser();

    /**
     * @return pointer to EDID data
     */
    ST_LOCAL const stUByte_t* getData() const {
        return myData;
    }

    /**
     * @return size of EDID data in bytes
     */
    ST_LOCAL unsigned int getSize() const {
        return mySize;
    }

    /**
     * @return number of extensions blocks (as encoded in first 128 block)
     */
    ST_LOCAL unsigned int getExtensionsNb() const {
        return myData != NULL ? myData[126] : 0;
    }

    /**
     * Initialize the parser.
     * @param theData pointer to EDID data
     * @param theSize size of data (should be at least 128 bytes long)
     */
    ST_CPPEXPORT void init(const stUByte_t*   theData,
                           const unsigned int theSize = 128);

    /**
     * Append extension blocks.
     * @param theData pointer to extension block(s)
     * @param theSize size of added data
     */
    ST_CPPEXPORT void add(const stUByte_t*   theData,
                          const unsigned int theSize);

    /**
     * @return true if given EDID data has valid header of EDID 1.x.
     */
    ST_CPPEXPORT bool isFirstVersion() const;

    /**
     * Returns true if checksum is valid.
     */
    ST_CPPEXPORT bool isValid() const;

    /**
     * Update checksum.
     */
    ST_CPPEXPORT void validate();

        public: //! @name method to retrieve information from EDID (isValid() should be called before!)

    /**
     * Returns EDID version number.
     */
    ST_CPPEXPORT unsigned int getVersion() const;

    /**
     * Returns EDID revision number.
     */
    ST_CPPEXPORT unsigned int getRevision() const;

    /**
     * Year of Manufacture.
     */
    ST_CPPEXPORT unsigned int getYear() const;

    /**
     * Week of Manufacture. This varies by manufacturer.
     * One way is to count January 1–7 as week 1, January 8–15 as week 2 and so on.
     * Some count based on the week number (Sunday-Saturday). Valid range is 1-54.
     */
    ST_CPPEXPORT unsigned int getWeek() const;

    /**
     * @return display gamma
     */
    ST_CPPEXPORT double getGamma() const;

    /**
     * @return the model name
     */
    ST_CPPEXPORT StString getName() const;

    /**
     * Parse the data and extract PnPID.
     */
    ST_CPPEXPORT StString getPnPId() const;

    /**
     * Setup new PnPID in EDID data.
     */
    ST_CPPEXPORT void setPnPId(const StString& thePnPIdString);

    /**
     * Retrieve the stereo flag.
     */
    ST_CPPEXPORT stEdid1Stereo_t getStereoFlag() const;
    ST_CPPEXPORT StString getStereoString() const;

    /**
     * @return display width in MM
     */
    ST_CPPEXPORT double getWidthMM() const;

    /**
     * @return display height in MM
     */
    ST_CPPEXPORT double getHeightMM() const;

        public:

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

        private: //! @name private fields

    stUByte_t*   myData; //!< EDID data
    unsigned int mySize; //!< data size in bytes

};

#endif //__StEDIDParser_h_
