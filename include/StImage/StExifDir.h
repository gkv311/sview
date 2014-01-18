/**
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StExifDir_h_
#define __StExifDir_h_

#include "StExifEntry.h"

#include <StAlienData.h>

#include <StTemplates/StHandle.h>
#include <StTemplates/StArrayList.h>

/**
 * Exif directory (Exchangeable Image File Format).
 */
class StExifDir {

        private: //!< retrieve values functions

    /**
     * Retrieve uint16_t value.
     */
    inline uint16_t get16u(const unsigned char* theShort) const {
        return myIsFileBE ? StAlienData::Get16uBE(theShort) : StAlienData::Get16uLE(theShort);
    }

    /**
     * Retrieve int32_t value.
     */
    inline int32_t get32s(const unsigned char* theLong) const {
        return myIsFileBE ? StAlienData::Get32sBE(theLong) : StAlienData::Get32sLE(theLong);
    }

    /**
     * Retrieve uint32_t value.
     */
    inline uint32_t get32u(const unsigned char* theLong) const {
        return myIsFileBE ? StAlienData::Get32uBE(theLong) : StAlienData::Get32uLE(theLong);
    }

    /**
     * Read the EXIF directory and its subdirectories.
     */
    ST_CPPEXPORT bool readDirectory(unsigned char* theDirStart, unsigned char* theOffsetBase,
                                    const size_t theExifLength, int theNestingLevel);

    /**
     * Read one entry in the EXIF directory.
     */
    ST_CPPEXPORT bool readEntry(unsigned char* theEntryAddress,
                                unsigned char* theOffsetBase,
                                const size_t   theExifLength,
                                StExifEntry&   theEntry);

    /**
     * Get pointer to the entry.
     */
    ST_CPPEXPORT unsigned char* getEntryAddress(const size_t theEntryId) const;

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StExifDir(bool theIsFileBE    = false,
                           bool theIsMakerNote = false);

    inline bool isFileBE() const {
        return myIsFileBE;
    }

    /**
     * Camera maker identificator.
     * In generic it could be editing software identificator.
     */
    ST_LOCAL const StString& getCameraMaker() const {
        return myCameraMaker;
    }

    /**
     * @return camera model
     */
    ST_LOCAL const StString& getCameraModel() const {
        return myCameraModel;
    }

    /**
     * @return user comment
     */
    ST_LOCAL const StString& getUserComment() const {
        return myUserComment;
    }

    /**
     * Clear current content of the directory.
     */
    ST_CPPEXPORT void reset();

    /**
     * Read the EXIF.
     */
    ST_CPPEXPORT bool parseExif(unsigned char* theExifSection,
                                const size_t   theLength);

    /**
     * Find entry by tag in current directory and subdirectories.
     * @param theIsMakerNote search for maker note tag or general tag
     * @param theEntry       the entry to find (tag field should be set)
     * @param theIsBigEndian the endianless in which entry was stored
     * @return true if entry was found
     */
    ST_CPPEXPORT bool findEntry(const bool   theIsMakerNote, // probably we can use makernote string here...
                                StExifEntry& theEntry,
                                bool&        theIsBigEndian) const;

    /**
     * @return subdirectories list
     */
    ST_LOCAL const StArrayList< StHandle<StExifDir> >& getSubDirs() const {
        return mySubDirs;
    }

        private:

    StArrayList< StHandle<StExifDir> > mySubDirs; //!< subdirectories list
    StArrayList< StExifEntry > myEntries;         //!< entries list
    StString       myCameraMaker; //!< just useful identification strings
    StString       myCameraModel;
    StString       myUserComment; //!< UserComment text
    unsigned char* myStartPtr;    //!< start pointer in the memory
    bool           myIsFileBE;    //!< indicate that data in this EXIF directory stored in Big-Endian order
    bool           myIsMakerNote; //!< maker notes from different vendors may probably has overlapped tags ids

};

template<>
inline void StArray< StHandle<StExifDir> >::sort() {}

#endif // __StExifDir_h_
