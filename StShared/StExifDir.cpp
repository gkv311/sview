/**
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StImage/StExifDir.h>

#include <StStrings/StLogger.h>

/**
 * Known EXIF tags.
 */
enum {
    TAG_MAKE                  = 0x010F,
    TAG_MODEL                 = 0x0110,
    TAG_EXIF_OFFSET           = 0x8769,
    TAG_MAKER_NOTE            = 0x927C,
    TAG_USER_COMMENT          = 0x9286,
    TAG_INTEROP_OFFSET        = 0xA005,
};

const size_t StExifEntry::BYTES_PER_FORMAT[StExifEntry::NUM_FORMATS + 1] = {0, 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8};

StExifDir::StExifDir(bool theIsFileBE, bool theIsMakerNote)
: myStartPtr(NULL),
  myIsFileBE(theIsFileBE),
  myIsMakerNote(theIsMakerNote) {
    //
}

bool StExifDir::readEntry(unsigned char* theEntryAddress,
                          unsigned char* theOffsetBase,
                          const size_t   theExifLength,
                          StExifEntry&   theEntry) {
    if(theEntryAddress == NULL) {
        return false;
    }

    theEntry.Tag        = get16u(theEntryAddress);
    theEntry.Format     = get16u(theEntryAddress + 2);
    theEntry.Components = get32u(theEntryAddress + 4);

    // validate format
    if((theEntry.Format - 1) >= StExifEntry::NUM_FORMATS) {
        // (-1) catches illegal zero case as unsigned underflows to positive large.
        ST_DEBUG_LOG("StExifDir, Illegal number format " + theEntry.Format + " for tag "
                     + theEntry.Tag + " in Exif");
        return false;
    }

    // validate components number
    if((unsigned int )theEntry.Components > 0x10000) {
        ST_DEBUG_LOG("StExifDir, Too many components (" + theEntry.Components + ") for tag "
                     + theEntry.Tag + " in Exif");
        return false;
    }

    size_t aBytesCount = theEntry.getBytes();
    if(aBytesCount > 4) {
        size_t anOffsetVal = size_t(get32u(theEntryAddress + 8));
        // if its bigger than 4 bytes, the dir entry contains an offset.
        if(anOffsetVal + aBytesCount > theExifLength) {
            // bogus pointer offset and / or bytecount value
            ST_DEBUG_LOG("StExifDir, Illegal value pointer for tag " + theEntry.Tag + " in Exif");
            return false;
        }
        theEntry.ValuePtr = theOffsetBase + anOffsetVal;
    } else {
        // 4 bytes or less and value is in the dir entry itself
        theEntry.ValuePtr = theEntryAddress + 8;
    }
    return true;
}

unsigned char* StExifDir::getEntryAddress(const size_t theEntryId) const {
    return (myStartPtr != NULL) ? (myStartPtr + 2 + theEntryId * 12) : NULL;
}

void StExifDir::reset() {
    /// TODO (Kirill Gavrilov#1) implement
}

bool StExifDir::parseExif(unsigned char* theExifSection,
                          const size_t   theLength) {
    // check the EXIF header component
    static const unsigned char EXIF_HEADER[] = "Exif\0\0";
    if(!stAreEqual(theExifSection + 2, EXIF_HEADER, 6)) {
        ST_DEBUG_LOG("Incorrect Exif header");
        return false;
    }

    if(stAreEqual(theExifSection + 8, "II", 2)) {
        ST_DEBUG_LOG("Exif section in Little-Endian order");
        myIsFileBE = false;
    } else if(stAreEqual(theExifSection + 8, "MM", 2)) {
        ST_DEBUG_LOG("Exif section in Big-Endian order");
        myIsFileBE = true;
    } else {
        ST_DEBUG_LOG("Invalid Exif endianness marker");
        return false;
    }

    // check the next value for correctness.
    if(get16u(theExifSection + 10) != 0x2A) {
        ST_DEBUG_LOG("Invalid Exif start (1)");
        return false;
    }

    size_t aFirstOffset = size_t(get32u(theExifSection + 12));
    if(aFirstOffset < 8 || aFirstOffset > 16) {
        if(aFirstOffset < 16 || aFirstOffset > theLength - 16) {
            ST_DEBUG_LOG("invalid offset for first Exif IFD value");
            return false;
        }
        // usually set to 8, but other values valid too
        ST_DEBUG_LOG("Suspicious offset of first Exif IFD value");
    }

    // first directory starts 16 bytes in
    // all offset are relative to 8 bytes in
    return readDirectory(theExifSection + 8 + aFirstOffset, theExifSection + 8, theLength - 8, 0);
}

bool StExifDir::readDirectory(unsigned char* theDirStart, unsigned char* theOffsetBase,
                              const size_t theExifLength, int theNestingLevel) {
    if(theNestingLevel > 4) {
        ST_DEBUG_LOG("StExifDir, Maximum EXIF directory nesting exceeded (corrupt EXIF header)");
        return false;
    }

    myStartPtr = theDirStart;

    // read number of entries
    size_t anEntriesNb = size_t(get16u(myStartPtr));
    unsigned char* aDirEnd = getEntryAddress(anEntriesNb);

    /*ST_DEBUG_LOG("StExifDir, subdirectory " + size_t(theOffsetBase)
               + "; " + size_t(myStartPtr - theOffsetBase)
               + "; " + size_t(theExifLength)
               + "; entries= " + anEntriesNb);*/

    if(aDirEnd + 4 > (theOffsetBase + theExifLength)) {
        if(aDirEnd + 2 == theOffsetBase + theExifLength
        || aDirEnd     == theOffsetBase + theExifLength) {
            // version 1.3 of jhead would truncate a bit too much.
            // this also caught later on as well.
        } else {
            ST_DEBUG_LOG("StExifDir, Illegally sized Exif subdirectory (" + anEntriesNb + " entries)");
            reset();
            return false;
        }
    }

    // add all entries
    myEntries.initList(anEntriesNb); // reserve space for the list
    StExifEntry anEntry;
    for(size_t anEntryId = 0; anEntryId < anEntriesNb; ++anEntryId) {
        if(!readEntry(getEntryAddress(anEntryId),
                      theOffsetBase, theExifLength,
                      anEntry)) {
            continue;
        }
        myEntries.add(anEntry);

        switch(anEntry.Tag) {
            case TAG_EXIF_OFFSET:
            case TAG_INTEROP_OFFSET: {
                unsigned char* aSubdirStart = theOffsetBase + size_t(get32u(anEntry.ValuePtr));
                if(aSubdirStart < theOffsetBase
                || aSubdirStart > theOffsetBase + theExifLength) {
                    ST_DEBUG_LOG("StExifDir, Illegal EXIF or interop offset directory link");
                } else {
                    StHandle<StExifDir> aSubDir = new StExifDir(isFileBE());
                    aSubDir->myCameraMaker = myCameraMaker;
                    aSubDir->myCameraModel = myCameraModel;
                    if(aSubDir->readDirectory(aSubdirStart, theOffsetBase,
                                              theExifLength, theNestingLevel + 1)) {
                        mySubDirs.add(aSubDir);
                    }
                }
                break;
            }
            case TAG_MAKE: {
                if(anEntry.Format == StExifEntry::FMT_STRING) {
                    // NULL-terminated ASCII string
                    myCameraMaker = StString((char *)anEntry.ValuePtr);
                    ST_DEBUG_LOG("StExifDir, CameraMaker= " + myCameraMaker);
                }
                break;
            }
            case TAG_MODEL: {
                if(anEntry.Format == StExifEntry::FMT_STRING) {
                    // NULL-terminated ASCII string
                    myCameraModel = StString((char *)anEntry.ValuePtr);
                    ST_DEBUG_LOG("StExifDir, CameraModel= " + myCameraModel);
                }
                break;
            }
            case TAG_MAKER_NOTE: {
                // maker note (vendor-specific tags)
                /// TODO (Kirill Gavrilov#9) base offset may be wrong for some camera makers
                unsigned char* aSubdirStart = anEntry.ValuePtr;
                unsigned char* anOffsetBase = theOffsetBase;
                size_t anOffsetLimit = theExifLength;
                StHandle<StExifDir> aSubDir;
                if(myCameraMaker.isEquals(stCString("FUJIFILM"))) {
                    // the Fujifilm maker note appears to be in little endian byte order regardless of the rest of the file
                    aSubDir = new StExifDir(false, true);
                    // it seems that Fujifilm maker notes start with an ID string,
                    // followed by an IFD offset relative to the MakerNote tag
                    if(stAreEqual(anEntry.ValuePtr, "FUJIFILM", 8)) {
                        //ST_DEBUG_LOG("Fuji string ID found");
                        aSubdirStart += size_t(StAlienData::Get16uLE(anEntry.ValuePtr + 8)); // read Little-Endian, that is
                        // found experimental
                        anOffsetBase  = anEntry.ValuePtr;
                        anOffsetLimit = theOffsetBase + theExifLength - anOffsetBase;
                    }
                } else if(myCameraMaker.isStartsWith(stCString("OLYMP"))) {
                    aSubDir = new StExifDir(isFileBE(), true);
                    // it seems that Olympus maker notes start with an ID string
                    if(stAreEqual(anEntry.ValuePtr, "OLYMP", 5)) {
                        //ST_DEBUG_LOG("OLYMP string ID found");
                        aSubdirStart += 8; // here is really 8 bytes offset!
                    }
                } else if(myCameraMaker.isStartsWith(stCString("Canon"))) {
                    // it seems that Canon maker notes is always little endian
                    aSubDir = new StExifDir(false, true);
                }
                if(!aSubDir.isNull()) {
                    ST_DEBUG_LOG("StExifDir, reading " + myCameraMaker + " maker notes");
                    aSubDir->myCameraMaker = myCameraMaker;
                    aSubDir->myCameraModel = myCameraModel;
                    if(aSubdirStart < theOffsetBase
                    || aSubdirStart > theOffsetBase + theExifLength) {
                        ST_DEBUG_LOG("StExifDir, illegal maker notes offset directory link");
                    } else if(aSubDir->readDirectory(aSubdirStart, anOffsetBase,
                                                     anOffsetLimit, theNestingLevel + 1)) {
                        mySubDirs.add(aSubDir);
                    }
                } else {
                    ST_DEBUG_LOG("StExifDir, found unknown (" + myCameraMaker + ") maker notes");
                }
                break;
            }
            case TAG_USER_COMMENT: {
                // custom block with undefined type
                if(anEntry.getBytes() < 9) {
                    break;
                }

                if(stAreEqual(anEntry.ValuePtr, "ASCII\0\0\0", 8)) {
                    const char* aStart = (const char* )anEntry.ValuePtr + 8;
                    myUserComment = StString(aStart, anEntry.getBytes() - 8);
                    ST_DEBUG_LOG("StExifDir, UserComment= '" + myUserComment + "'");
                } else if(stAreEqual(anEntry.ValuePtr, "UNICODE\0",   8)) {
                    const char* aStart = (const char* )anEntry.ValuePtr + 8;
                    myUserComment = StString((stUtf16_t *)aStart, (anEntry.getBytes() - 8) / 2);
                    ST_DEBUG_LOG("StExifDir, UserComment= '" + myUserComment + "'");
                }
                break;
            }
        }
    }

    // in addition to linking to subdirectories via exif tags,
    // there's also a potential link to another directory at the end of each
    // directory.  this has got to be the result of a committee!
    if(getEntryAddress(anEntriesNb) + 4 <= theOffsetBase + theExifLength) {
        const size_t anOffset = size_t(get32u(myStartPtr + 2 + anEntriesNb * 12));
        if(anOffset > 0) {
            unsigned char* aSubdirStart = theOffsetBase + anOffset;
            if(aSubdirStart > theOffsetBase + theExifLength || aSubdirStart < theOffsetBase) {
                if(aSubdirStart > theOffsetBase
                && aSubdirStart < theOffsetBase + theExifLength + 20) {
                    //
                } else {
                    ST_DEBUG_LOG("StExifDir, Illegal subdirectory link in EXIF header");
                }
            } else {
                if(aSubdirStart <= theOffsetBase + theExifLength) {
                    // continued directory
                    StHandle<StExifDir> aSubDir = new StExifDir(isFileBE());
                    aSubDir->myCameraMaker = myCameraMaker;
                    aSubDir->myCameraModel = myCameraModel;
                    if(aSubDir->readDirectory(aSubdirStart, theOffsetBase,
                                              theExifLength, theNestingLevel + 1)) {
                        mySubDirs.add(aSubDir);
                    }
                }
            }
        }
    }
    ST_DEBUG_LOG("StExifDir, subdir level= " + theNestingLevel
               + " entries number= " + myEntries.size());
    return true;
}

bool StExifDir::findEntry(const bool   theIsMakerNote,
                          StExifEntry& theEntry,
                          bool&        theIsBigEndian) const {
    // search own entries
    if(!(theIsMakerNote ^ myIsMakerNote)) {
        for(size_t anEntryId = 0; anEntryId < myEntries.size(); ++anEntryId) {
            const StExifEntry& anEntry = myEntries[anEntryId];
            if(anEntry.Tag == theEntry.Tag) {
                theEntry       = anEntry;
                theIsBigEndian = myIsFileBE;
                return true;
            }
        }
    }
    // search in subfolders
    for(size_t aDirId = 0; aDirId < mySubDirs.size(); ++aDirId) {
        const StHandle<StExifDir>& aDir = mySubDirs[aDirId];
        if(!aDir.isNull()
         && aDir->findEntry(theIsMakerNote, theEntry, theIsBigEndian)) {
            return true;
        }
    }
    return false;
}
