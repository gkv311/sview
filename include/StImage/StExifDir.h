/**
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StExifDir_h_
#define __StExifDir_h_

#include "StExifEntry.h"

#include <StAlienData.h>

#include <StTemplates/StHandle.h>
#include <StTemplates/StArrayList.h>

class StDictionary;

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

    /**
     * Structure for search request.
     */
    struct Query {
        DirType          Type;   //!< directory filter
        StExifEntry      Entry;  //!< entry
        const StExifDir* Folder; //!< (sub) folder containing the entry

        Query() : Type(DType_General), Folder(NULL) {
            Entry.ValuePtr   = NULL;
            Entry.Tag        = 0;
            Entry.Format     = 0;
            Entry.Components = 0;
        }

        Query(DirType theType, uint16_t theTag, uint16_t theFormat = 0)
        : Type(theType), Folder(NULL) {
            Entry.ValuePtr   = NULL;
            Entry.Tag        = theTag;
            Entry.Format     = theFormat;
            Entry.Components = 0;
        }
    };

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

    ST_CPPEXPORT void format(const StExifEntry& theEntry,
                             StString&          theString) const;

    /**
     * Fill dictionary with known tags.
     */
    ST_CPPEXPORT void fillDictionary(StDictionary& theDict,
                                     const bool    theToShowUnknown) const;

    /**
     * Find entry by tag. On search fail theQuery will be left untouched.
     * @param theList  directory list
     * @param theQuery the search query with specified tag and directory filter, will be filled with other data on success
     * @return true if entry was found
     */
    ST_CPPEXPORT static bool findEntry(const StExifDir::List& theList,
                                       StExifDir::Query&      theQuery);

        public: //! @name retrieve values functions

    inline   int8_t  get8s(const stUByte_t* theByte)  const { return StAlienData::Get8s(theByte); }
    inline  uint8_t  get8u(const stUByte_t* theByte)  const { return StAlienData::Get8u(theByte); }
    inline  int16_t get16s(const stUByte_t* theShort) const { return StAlienData::Get16s(theShort, IsFileBE); }
    inline uint16_t get16u(const stUByte_t* theShort) const { return StAlienData::Get16u(theShort, IsFileBE); }
    inline  int32_t get32s(const stUByte_t* theLong)  const { return StAlienData::Get32s(theLong,  IsFileBE); }
    inline uint32_t get32u(const stUByte_t* theLong)  const { return StAlienData::Get32u(theLong,  IsFileBE); }

        private:

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
