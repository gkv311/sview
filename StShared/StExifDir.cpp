/**
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StImage/StExifDir.h>
#include <StImage/StExifTags.h>

#include <StStrings/StDictionary.h>
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

StExifDir::StExifDir()
: Type(StExifDir::DType_General),
  IsFileBE(true),
  myStartPtr(NULL) {
    //
}

bool StExifDir::readEntry(stUByte_t*   theEntryAddress,
                          stUByte_t*   theOffsetBase,
                          const size_t theExifLength,
                          StExifEntry& theEntry) {
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

    const size_t aBytesCount = theEntry.getBytes();
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

bool StExifDir::parseExif(StExifDir::List& theParentList,
                          stUByte_t*       theExifSection,
                          const size_t     theLength) {
    if(theLength < 10) {
        ST_DEBUG_LOG("StExifDir, wrong length " + theLength);
        return false;
    }

    if(stAreEqual(theExifSection, "II", 2)) {
        //ST_DEBUG_LOG("Exif section in Little-Endian order");
        IsFileBE = false;
    } else if(stAreEqual(theExifSection, "MM", 2)) {
        //ST_DEBUG_LOG("Exif section in Big-Endian order");
        IsFileBE = true;
    } else {
        ST_DEBUG_LOG("Invalid Exif endianness marker");
        return false;
    }

    // check the next value for correctness.
    if(get16u(theExifSection + 2) != 0x2A) {
        ST_DEBUG_LOG("Invalid Exif start (1)");
        return false;
    }

    const size_t aFirstOffset = size_t(get32u(theExifSection + 4));
    if(aFirstOffset < 8
    || aFirstOffset >= theLength - 8 - 2) {
        ST_DEBUG_LOG("StExifDir, invalid offset " + aFirstOffset + " for first Exif IFD value");
        return false;
    }
    if(aFirstOffset > 16) {
        // usually set to 8, but other values valid too
        ST_DEBUG_LOG("StExifDir, suspicious offset of first Exif IFD value");
    }

    // first directory starts 16 bytes in
    // all offset are relative to 8 bytes in
    return readDirectory(theParentList, theExifSection + aFirstOffset, theExifSection, theLength, 0);
}

bool StExifDir::readDirectory(StExifDir::List& theParentList,
                              stUByte_t*       theDirStart,
                              stUByte_t*       theOffsetBase,
                              const size_t     theExifLength,
                              const int        theNestingLevel) {
    if(theNestingLevel > 4) {
        ST_DEBUG_LOG("StExifDir, Maximum EXIF directory nesting exceeded (corrupt EXIF header)");
        return false;
    }

    myStartPtr = theDirStart;

    // read number of entries
    const uint16_t   anEntriesNb = get16u(myStartPtr);
    const stUByte_t* aDirEnd     = getEntryAddress(anEntriesNb);

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
            return false;
        }
    }

    // add all entries
    Entries.initList((size_t )anEntriesNb); // reserve space for the list
    StExifEntry anEntry;
    for(uint16_t anEntryId = 0; anEntryId < anEntriesNb; ++anEntryId) {
        if(!readEntry(getEntryAddress(anEntryId),
                      theOffsetBase, theExifLength,
                      anEntry)) {
            continue;
        }
        Entries.add(anEntry);

        switch(anEntry.Tag) {
            case TAG_EXIF_OFFSET:
            case TAG_INTEROP_OFFSET: {
                stUByte_t* aSubdirStart = theOffsetBase + size_t(get32u(anEntry.ValuePtr));
                if(aSubdirStart < theOffsetBase
                || aSubdirStart > theOffsetBase + theExifLength) {
                    ST_DEBUG_LOG("StExifDir, Illegal EXIF or interop offset directory link");
                } else {
                    StHandle<StExifDir> aSubDir = new StExifDir();
                    aSubDir->IsFileBE    = IsFileBE;
                    aSubDir->CameraMaker = CameraMaker;
                    aSubDir->CameraModel = CameraModel;
                    SubDirs.add(aSubDir);
                    if(!aSubDir->readDirectory(SubDirs,
                                               aSubdirStart, theOffsetBase,
                                               theExifLength, theNestingLevel + 1)) {
                        //
                    }
                }
                break;
            }
            case TAG_MAKE: {
                if(anEntry.Format == StExifEntry::FMT_STRING) {
                    // NULL-terminated ASCII string
                    CameraMaker = StString((char *)anEntry.ValuePtr);
                    ST_DEBUG_LOG("StExifDir, CameraMaker= " + CameraMaker);
                }
                break;
            }
            case TAG_MODEL: {
                if(anEntry.Format == StExifEntry::FMT_STRING) {
                    // NULL-terminated ASCII string
                    CameraModel = StString((char *)anEntry.ValuePtr);
                    ST_DEBUG_LOG("StExifDir, CameraModel= " + CameraModel);
                }
                break;
            }
            case TAG_MAKER_NOTE: {
                // maker note (vendor-specific tags)
                /// TODO (Kirill Gavrilov#9) base offset may be wrong for some camera makers
                stUByte_t* aSubdirStart  = anEntry.ValuePtr;
                stUByte_t* anOffsetBase  = theOffsetBase;
                size_t     anOffsetLimit = theExifLength;
                StHandle<StExifDir> aSubDir;
                if(CameraMaker.isEquals(stCString("FUJIFILM"))) {
                    // the Fujifilm maker note appears to be in little endian byte order regardless of the rest of the file
                    aSubDir = new StExifDir();
                    aSubDir->IsFileBE = false;
                    aSubDir->Type     = DType_MakerFuji;
                    // it seems that Fujifilm maker notes start with an ID string,
                    // followed by an IFD offset relative to the MakerNote tag
                    if(stAreEqual(anEntry.ValuePtr, "FUJIFILM", 8)) {
                        aSubdirStart += size_t(StAlienData::Get16uLE(anEntry.ValuePtr + 8)); // read Little-Endian, that is
                        // found experimental
                        anOffsetBase  = anEntry.ValuePtr;
                        anOffsetLimit = theOffsetBase + theExifLength - anOffsetBase;
                    }
                } else if(CameraMaker.isStartsWith(stCString("OLYMP"))) {
                    aSubDir = new StExifDir();
                    aSubDir->IsFileBE = IsFileBE;
                    aSubDir->Type     = DType_MakerOlypm;
                    // it seems that Olympus maker notes start with an ID string
                    if(stAreEqual(anEntry.ValuePtr, "OLYMP", 5)) {
                        //ST_DEBUG_LOG("OLYMP string ID found");
                        aSubdirStart += 8; // here is really 8 bytes offset!
                    }
                } else if(CameraMaker.isStartsWith(stCString("Canon"))) {
                    // it seems that Canon maker notes is always little endian
                    aSubDir = new StExifDir();
                    aSubDir->IsFileBE = false;
                    aSubDir->Type     = DType_MakerCanon;
                }
                if(!aSubDir.isNull()) {
                    ST_DEBUG_LOG("StExifDir, reading " + CameraMaker + " maker notes");
                    aSubDir->CameraMaker = CameraMaker;
                    aSubDir->CameraModel = CameraModel;
                    if(aSubdirStart < theOffsetBase
                    || aSubdirStart > theOffsetBase + theExifLength) {
                        ST_DEBUG_LOG("StExifDir, illegal maker notes offset directory link");
                    } else {
                        SubDirs.add(aSubDir);
                        if(!aSubDir->readDirectory(SubDirs,
                                                   aSubdirStart, anOffsetBase,
                                                   anOffsetLimit, theNestingLevel + 1)) {
                            //
                        }
                    }
                } else {
                    if(anEntry.Format == StExifEntry::FMT_STRING) {
                        StString aMakerNoteName((char *)anEntry.ValuePtr);
                        ST_DEBUG_LOG("StExifDir, found unsupported (" + aMakerNoteName + ") maker notes");
                    } else {
                        ST_DEBUG_LOG("StExifDir, found unsupported (" + CameraMaker + ": " + CameraModel + ") maker notes");
                    }
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
                    UserComment = StString(aStart, anEntry.getBytes() - 8);
                    ST_DEBUG_LOG("StExifDir, UserComment= '" + UserComment + "'");
                } else if(stAreEqual(anEntry.ValuePtr, "UNICODE\0",   8)) {
                    const char* aStart = (const char* )anEntry.ValuePtr + 8;
                    UserComment = StString((stUtf16_t *)aStart, (anEntry.getBytes() - 8) / 2);
                    ST_DEBUG_LOG("StExifDir, UserComment= '" + UserComment + "'");
                }
                break;
            }
        }
    }

    // in addition to linking to subdirectories via exif tags,
    // there's also a potential link to another directory at the end of each directory.
    if(getEntryAddress(anEntriesNb) + 4 <= theOffsetBase + theExifLength) {
        const size_t anOffset = size_t(get32u(myStartPtr + 2 + anEntriesNb * 12));
        if(anOffset != 0) {
            stUByte_t* aSubdirStart = theOffsetBase + anOffset;
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
                    StHandle<StExifDir> aSubDir = new StExifDir();
                    aSubDir->Type        = Type;
                    aSubDir->IsFileBE    = IsFileBE;
                    aSubDir->CameraMaker = CameraMaker;
                    aSubDir->CameraModel = CameraModel;
                    theParentList.add(aSubDir);
                    if(!aSubDir->readDirectory(theParentList,
                                               aSubdirStart, theOffsetBase,
                                               theExifLength, theNestingLevel)) {
                        //
                    }
                }
            }
        }
    }
    //ST_DEBUG_LOG("StExifDir, subdir level= " + theNestingLevel
    //           + " entries number= " + myEntries.size());
    return true;
}

void StExifDir::format(const StExifEntry& theEntry,
                       StString&          theString) const {
    switch(theEntry.Format) {
        case StExifEntry::FMT_BYTE: {
            theString = get8u(theEntry.ValuePtr);
            return;
        }
        case StExifEntry::FMT_STRING: {
            theString = StString((char *)theEntry.ValuePtr);
            return;
        }
        case StExifEntry::FMT_USHORT: {
            theString = get16u(theEntry.ValuePtr);
            return;
        }
        case StExifEntry::FMT_ULONG: {
            theString = get32u(theEntry.ValuePtr);
            return;
        }
        case StExifEntry::FMT_URATIONAL: {
            const uint32_t aNum = get32u(theEntry.ValuePtr);
            const uint32_t aDen = get32u(theEntry.ValuePtr + 4);
            theString = StString() + aNum + "/" + aDen;
            if(aDen != 0
            && aDen != 1) {
                theString += StString(" (") + (double(aNum) / double(aDen)) + ")";
            }
            return;
        }
        case StExifEntry::FMT_SBYTE: {
            theString = get8s(theEntry.ValuePtr);
            return;
        }
        case StExifEntry::FMT_SSHORT: {
            theString = get16s(theEntry.ValuePtr);
            return;
        }
        case StExifEntry::FMT_SLONG: {
            theString = get32s(theEntry.ValuePtr);
            return;
        }
        case StExifEntry::FMT_SRATIONAL: {
            const int32_t aNum = get32s(theEntry.ValuePtr);
            const int32_t aDen = get32s(theEntry.ValuePtr + 4);
            theString = StString() + aNum + "/" + aDen;
            if(aDen != 0
            && aDen != 1) {
                theString += StString(" (") + (double(aNum) / double(aDen)) + ")";
            }
            return;
        }
        case StExifEntry::FMT_SINGLE: {
            theString = StString(*(float* )theEntry.ValuePtr);
            return;
        }
        case StExifEntry::FMT_DOUBLE: {
            theString = StString(*(double* )theEntry.ValuePtr);
            return;
        }
        case StExifEntry::FMT_UNDEFINED:
        default: {
            theString = stCString("N/A");
            return;
        }
    }
}

inline void formatTag(const uint16_t theTag,
                      char*          theString) {
    stsprintf(theString, 5, "%04X", theTag);
}

void StExifDir::fillDictionary(StDictionary& theDict,
                               const bool    theToShowUnknown) const {
    using namespace StExifTags;
    char aTagHex[8];
    StExifTagsMap aMap;
    switch(Type) {
        case DType_General: {
            for(size_t anEntryId = 0; anEntryId < Entries.size(); ++anEntryId) {
                const StExifEntry& anEntry = Entries[anEntryId];
                const StExifTag*   aTagDef = aMap.findImageTag(anEntry.Tag);
                if(aTagDef != NULL) {
                    format(anEntry, theDict.addChange(aTagDef->Name).changeValue());
                } else if(theToShowUnknown) {
                    formatTag(anEntry.Tag, aTagHex);
                    format(anEntry, theDict.addChange(StString("Exif.Image.") + aTagHex).changeValue());
                }
            }
            break;
        }
        case DType_MakerOlypm: {
            for(size_t anEntryId = 0; anEntryId < Entries.size(); ++anEntryId) {
                const StExifEntry& anEntry = Entries[anEntryId];
                const StExifTag*   aTagDef = aMap.findOlympTag(anEntry.Tag);
                if(aTagDef != NULL) {
                    format(anEntry, theDict.addChange(aTagDef->Name).changeValue());
                } else if(theToShowUnknown) {
                    formatTag(anEntry.Tag, aTagHex);
                    format(anEntry, theDict.addChange(StString("Exif.Olympus.") + aTagHex).changeValue());
                }
            }
            break;
        }
        case DType_MakerCanon: {
            for(size_t anEntryId = 0; anEntryId < Entries.size(); ++anEntryId) {
                const StExifEntry& anEntry = Entries[anEntryId];
                const StExifTag*   aTagDef = aMap.findCanonTag(anEntry.Tag);
                if(aTagDef != NULL) {
                    format(anEntry, theDict.addChange(aTagDef->Name).changeValue());
                } else if(theToShowUnknown) {
                    formatTag(anEntry.Tag, aTagHex);
                    format(anEntry, theDict.addChange(StString("Exif.Canon.") + aTagHex).changeValue());
                }
            }
            break;
        }
        case DType_MakerFuji: {
            for(size_t anEntryId = 0; anEntryId < Entries.size(); ++anEntryId) {
                const StExifEntry& anEntry = Entries[anEntryId];
                const StExifTag*   aTagDef = aMap.findFujiTag(anEntry.Tag);
                if(aTagDef != NULL) {
                    format(anEntry, theDict.addChange(aTagDef->Name).changeValue());
                } else if(theToShowUnknown) {
                    formatTag(anEntry.Tag, aTagHex);
                    format(anEntry, theDict.addChange(StString("Exif.Fujifilm.") + aTagHex).changeValue());
                }
            }
            break;
        }
        case DType_MPO: {
            for(size_t anEntryId = 0; anEntryId < Entries.size(); ++anEntryId) {
                const StExifEntry& anEntry = Entries[anEntryId];
                const StExifTag*   aTagDef = aMap.findMpoTag(anEntry.Tag);
                if(aTagDef != NULL) {
                    format(anEntry, theDict.addChange(aTagDef->Name).changeValue());
                } else if(theToShowUnknown) {
                    formatTag(anEntry.Tag, aTagHex);
                    format(anEntry, theDict.addChange(StString("Exif.MP.") + aTagHex).changeValue());
                }
            }
            break;
        }
    }

    for(size_t aDirId = 0; aDirId < SubDirs.size(); ++aDirId) {
        const StHandle<StExifDir>& aDir = SubDirs[aDirId];
        if(!aDir.isNull()) {
            aDir->fillDictionary(theDict, theToShowUnknown);
        }
    }
}

bool StExifDir::findEntry(const StExifDir::List& theList,
                          StExifDir::Query&      theQuery) {
    // search in subfolders
    for(size_t aDirId = 0; aDirId < theList.size(); ++aDirId) {
        const StHandle<StExifDir>& aDir = theList[aDirId];
        if(aDir.isNull()) {
            continue;
        }

        if(aDir->Type == theQuery.Type) {
            for(size_t anEntryId = 0; anEntryId < aDir->Entries.size(); ++anEntryId) {
                const StExifEntry& anEntry = aDir->Entries[anEntryId];
                if(anEntry.Tag == theQuery.Entry.Tag
                && (anEntry.Format == theQuery.Entry.Format || theQuery.Entry.Format == 0)) {
                    theQuery.Entry  = anEntry;
                    theQuery.Folder = aDir.access();
                    return true;
                }
            }
        }
        if(findEntry(aDir->SubDirs, theQuery)) {
            return true;
        }
    }
    return false;
}
