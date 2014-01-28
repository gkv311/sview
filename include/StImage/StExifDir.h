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

        public:

    /**
     * Directory type.
     */
    enum DirType {
        DType_General,    //!< general EXIF directory
        //DType_MakerNote,  //!< Maker Note sub-directory (unsupported vendor)
        DType_MakerOlypm, //!< Maker Note sub-directory ("OLYMP")
        DType_MakerCanon, //!< Maker Note sub-directory ("Canon")
        DType_MakerFuji,  //!< Maker Note sub-directory ("FUJIFILM")
        DType_MPO,        //!< MP extensions
    };

    typedef StArrayList< StHandle<StExifDir> > List;

        public:

    StExifDir::List           SubDirs; //!< subdirectories list
    StArrayList<StExifEntry>  Entries; //!< entries list

    DirType  Type;        //!< tags from different vendors/extensions may has overlapped ids
    bool     IsFileBE;    //!< indicate that data in this EXIF directory stored in Big-Endian order
    StString CameraMaker; //!< Camera maker identificator. In generic it could be editing software identificator.
    StString CameraModel;
    StString UserComment; //!< UserComment text

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StExifDir();

    /**
     * Read the EXIF.
     */
    ST_CPPEXPORT bool parseExif(StExifDir::List& theParentList,
                                stUByte_t*       theExifSection,
                                const size_t     theLength);

    /**
     * Find entry by tag in current directory and subdirectories.
     * @param theDirType     directory type filter
     * @param theEntry       the entry to find (tag field should be set)
     * @param theIsBigEndian the endianless in which entry was stored
     * @return true if entry was found
     */
    ST_CPPEXPORT bool findEntry(const DirType theDirType,
                                StExifEntry&  theEntry,
                                bool&         theIsBigEndian) const;

        private: //! @name retrieve values functions

    inline uint16_t get16u(const stUByte_t* theShort) const { return StAlienData::Get16u(theShort, IsFileBE); }
    inline int32_t  get32s(const stUByte_t* theLong)  const { return StAlienData::Get32s(theLong,  IsFileBE); }
    inline uint32_t get32u(const stUByte_t* theLong)  const { return StAlienData::Get32u(theLong,  IsFileBE); }

    /**
     * Read the EXIF directory and its subdirectories.
     */
    ST_CPPEXPORT bool readDirectory(StExifDir::List& theParentList,
                                    stUByte_t*       theDirStart,
                                    stUByte_t*       theOffsetBase,
                                    const size_t     theExifLength,
                                    const int        theNestingLevel);

    /**
     * Read one entry in the EXIF directory.
     */
    ST_CPPEXPORT bool readEntry(stUByte_t*   theEntryAddress,
                                stUByte_t*   theOffsetBase,
                                const size_t theExifLength,
                                StExifEntry& theEntry);

    /**
     * Get pointer to the entry.
     */
    ST_LOCAL stUByte_t* getEntryAddress(const uint16_t theEntryId) const {
        return myStartPtr + 2 + size_t(theEntryId) * 12;
    }

        private:

    stUByte_t* myStartPtr; //!< start pointer in the memory

};

template<>
inline void StArray< StHandle<StExifDir> >::sort() {}

#endif // __StExifDir_h_
