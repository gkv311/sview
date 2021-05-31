/**
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <cstdio> // forward-declared to work-around broken MinGW headers

#include <StImage/StJpegParser.h>
#include <StImage/StExifTags.h>

#include <StStrings/StLogger.h>

/**
 * JPEG markers consist of one or more 0xFF bytes, followed by a marker
 * code byte (which is not an 0xFF).
 */
enum {
    M_SOF0  = 0xC0, // Start Of Frame N
    M_SOF1  = 0xC1, // N indicates which compression process
    M_SOF2  = 0xC2, // only SOF0-SOF2 are now in common use
    M_SOF3  = 0xC3,
    M_SOF5  = 0xC5, // NB: codes C4 and CC are NOT SOF markers
    M_SOF6  = 0xC6,
    M_SOF7  = 0xC7,
    M_SOF9  = 0xC9,
    M_SOF10 = 0xCA,
    M_SOF11 = 0xCB,
    M_SOF13 = 0xCD,
    M_SOF14 = 0xCE,
    M_SOF15 = 0xCF,

    M_DHT   = 0xC4, // Huffman table definition

    M_DAC   = 0xCC, // define arithmetic-coding conditioning

    M_RST0  = 0xD0, // restart with modulo 8 count "m"
    M_RST1  = 0xD1,
    M_RST2  = 0xD2,
    M_RST3  = 0xD3,
    M_RST4  = 0xD4,
    M_RST5  = 0xD5,
    M_RST6  = 0xD6,
    M_RST7  = 0xD7,

    M_SOI   = 0xD8, // Start Of Image (beginning of datastream)
    M_EOI   = 0xD9, // End Of Image (end of datastream)
    M_SOS   = 0xDA, // Start Of Scan (begins compressed data)
    M_DQT   = 0xDB, // define quantization tables
    M_DNL   = 0xDC, // define number of lines
    M_DRI   = 0xDD, // define restart interval
    M_DHP   = 0xDE, // define hierarchical progression
    M_EXP   = 0xDF, // expand reference components

    M_JFIF  = 0xE0, // Jfif marker (APP0)
    M_EXIF  = 0xE1, // Exif attribute information (APP1)
    M_APP2  = 0xE2, // Exif extended data
    M_APP3  = 0xE3, // _JPSJPS_
    M_APP4  = 0xE4,
    M_APP5  = 0xE5,
    M_APP6  = 0xE6,
    M_APP7  = 0xE7,
    M_APP8  = 0xE8,
    M_APP9  = 0xE9,
    M_APP10 = 0xEA,
    M_APP11 = 0xEB,
    M_APP12 = 0xEC,
    M_APP13 = 0xED,
    M_IPTC  = M_APP13, // IPTC marker
    M_APP14 = 0xEE,
    M_APP15 = 0xEF,

    M_COM   = 0xFE, // COMment

};

namespace {
    inline StString markerString(const int theMarker) {
        switch(theMarker) {
            case M_SOF0:  return stCString("SOF0 ");
            case M_SOF1:  return stCString("SOF1 ");
            case M_SOF2:  return stCString("SOF2 ");
            case M_SOF3:  return stCString("SOF3 ");
            case M_SOF5:  return stCString("SOF5 ");
            case M_SOF6:  return stCString("SOF6 ");
            case M_SOF7:  return stCString("SOF6 ");
            case M_SOF9:  return stCString("SOF9 ");
            case M_SOF10: return stCString("SOF10");
            case M_SOF11: return stCString("SOF11");
            case M_SOF13: return stCString("SOF13");
            case M_SOF14: return stCString("SOF14");
            case M_SOF15: return stCString("SOF15");

            case M_DHT:   return stCString("DHT  ");

            case M_DAC:   return stCString("DAC  ");

            case M_RST0:  return stCString("RST0 ");
            case M_RST1:  return stCString("RST1 ");
            case M_RST2:  return stCString("RST2 ");
            case M_RST3:  return stCString("RST3 ");
            case M_RST4:  return stCString("RST4 ");
            case M_RST5:  return stCString("RST5 ");
            case M_RST6:  return stCString("RST6 ");
            case M_RST7:  return stCString("RST7 ");

            case M_SOI:   return stCString("SOI  ");
            case M_EOI:   return stCString("EOI  ");
            case M_SOS:   return stCString("SOS  ");
            case M_DQT:   return stCString("DQT  ");
            case M_DNL:   return stCString("DNL  ");
            case M_DRI:   return stCString("DRI  ");
            case M_DHP:   return stCString("DHP  ");
            case M_EXP:   return stCString("EXP  ");

            case M_JFIF:  return stCString("JFIF ");
            case M_EXIF:  return stCString("EXIF ");
            case M_APP2:  return stCString("APP2 ");
            case M_APP3:  return stCString("APP3 ");
            case M_APP4:  return stCString("APP4 ");
            case M_APP5:  return stCString("APP5 ");
            case M_APP6:  return stCString("APP6 ");
            case M_APP7:  return stCString("APP7 ");
            case M_APP8:  return stCString("APP8 ");
            case M_APP9:  return stCString("APP9 ");
            case M_APP10: return stCString("APP10");
            case M_APP11: return stCString("APP11");
            case M_APP12: return stCString("APP12");
            case M_IPTC:  return stCString("IPTC ");
            case M_APP14: return stCString("APP14");
            case M_APP15: return stCString("APP15");

            case M_COM:   return stCString("COM  ");

            default:      return StString(theMarker) + "  ";
        }
    }
}

StJpegParser::StJpegParser(const StCString& theFilePath)
: StRawFile(theFilePath),
  myImages(NULL),
  myStFormat(StFormat_AUTO),
  myPanorama(StPanorama_OFF) {
    stMemZero(myOffsets, sizeof(myOffsets));
#if !defined(_MSC_VER)
    (void )markerString;
#endif
}

StJpegParser::~StJpegParser() {
    //
}

void StJpegParser::reset() {
    // destroy all images
    myImages.nullify();
    myComment.clear();
    myXMP.clear();
    myStFormat = StFormat_AUTO;
    myPanorama = StPanorama_OFF;
    myLength = 0;
    stMemZero(myOffsets, sizeof(myOffsets));
}

bool StJpegParser::readFile(const StCString& theFilePath,
                            const int        theOpenedFd,
                            const size_t     theReadMax) {
    reset();
    if(!StRawFile::readFile(theFilePath, theOpenedFd, theReadMax)) {
        return false;
    }

    myLength = myBuffSize;
    return parse();
}

bool StJpegParser::parse() {
    if(myBuffer == NULL) {
        return false;
    }

    int aCount = 0;
    myImages = parseImage(++aCount, 1, myBuffer, false);
    if(myImages.isNull()) {
        return false;
    }

    // continue reading the file (MPO may contains more than 1 image)
    for(StHandle<StJpegParser::Image> anImg = myImages;
        !anImg.isNull(); anImg = anImg->Next) {
        anImg->Next = parseImage(++aCount, 1, anImg->Data + anImg->Length, true);
    }

    return true;
}

StHandle<StJpegParser::Image> StJpegParser::parseImage(const int      theImgCount,
                                                       const int      theDepth,
                                                       unsigned char* theDataStart,
                                                       const bool     theToFindSOI) {
    // check out of bounds
    if(theDataStart == NULL) {
        return StHandle<StJpegParser::Image>();
    }

    unsigned char*       aData    = theDataStart;
    const unsigned char* aDataEnd = myBuffer + myLength;

    // search image beginning
    if(theToFindSOI) {
        ++aData;
        for(; aData < aDataEnd; ++aData) {
            if(aData[-1] == 0xFF && aData[0] == M_SOI) {
                --aData;
                break;
            }
        }
    }

    // check out of bounds
    if((aData + 2) > aDataEnd) {
        return StHandle<StJpegParser::Image>();
    }

    // check the jpeg identifier
    if(aData[0] != 0xFF || aData[1] != M_SOI) {
        ST_DEBUG_LOG("StJpegParser, no SOI at position " + size_t(aData - myBuffer) + " / " + myLength);
        return StHandle<StJpegParser::Image>();
    }
    aData += 2; // skip already read bytes

    // parse the data
    StHandle<StJpegParser::Image> anImg = new StJpegParser::Image();
    anImg->Data = aData - 2;
    bool toDetectCubemap = false;

    for(;;) {
        // search for the next marker in the file
        ++aData; // one byte forward
        size_t aSkippedBytes = 0;
        unsigned char aMarker = 0;
        for(; aData < aDataEnd; ++aSkippedBytes, ++aData) {
            aMarker = aData[0];
            if(aData[-1] == 0xFF
            && aMarker   != 0xFF
            && aMarker   != 0x00) {
                ++aData; // skip marker id byte
                break;
            }
        }

        //ST_DEBUG_LOG(" #" + theImgCount + "." + theDepth + " [" + markerString(aMarker) + "] at position " + size_t(aData - myBuffer) + " / " + myLength); ///
        if(aMarker == M_EOI) {
            //ST_DEBUG_LOG("Jpeg, EOI at position " + size_t(aData - myBuffer) + " / " + myLength);

            bool isPanoStereo = false;
            if(toDetectCubemap && myPanorama == StPanorama_OFF) {
                size_t aViewX = anImg->SizeX, aViewY = anImg->SizeY;
                if(myStFormat == StFormat_SideBySide_LR || myStFormat == StFormat_SideBySide_RL) {
                    aViewX /= 2;
                } else if(myStFormat == StFormat_TopBottom_LR || myStFormat == StFormat_TopBottom_RL) {
                    aViewY /= 2;
                }
                if(aViewX == aViewY * 6) {
                    myPanorama = StPanorama_Cubemap6_1;
                } else if(aViewX * 6 == aViewY) {
                    myPanorama = StPanorama_Cubemap1_6;
                } else if(aViewX * 2 == aViewY * 3) {
                    myPanorama = StPanorama_Cubemap3_2;
                }
            } else if(anImg->get360PanoMakerNote(isPanoStereo)) {
                if(myPanorama == StPanorama_OFF) {
                    myPanorama = StPanorama_Sphere;
                }
                if(myStFormat == StFormat_AUTO
                && isPanoStereo) {
                    myStFormat = StFormat_TopBottom_LR;
                }
            }

            anImg->Length = size_t(aData - anImg->Data);
            return anImg;
        } else if(aMarker == M_SOI) {
            // here the subimage (thumbnail)...
            //ST_DEBUG_LOG("Jpeg, SOI at position " + size_t(aData - myBuffer) + " / " + myLength);
            anImg->Thumb = StJpegParser::parseImage(theImgCount, theDepth + 1, aData - 2, false);
            if(!anImg->Thumb.isNull()) {
                //ST_DEBUG_LOG("anImg->Thumb->Length= " + anImg->Thumb->Length);
                aData += anImg->Thumb->Length - 2;
            }
            continue;
        }

        if(aData + 2 >= aDataEnd) {
            ST_DEBUG_LOG("Corrupt jpeg file or error in parser");
            if(myImages.isNull()) {
                anImg->Data   = myBuffer;
                anImg->Length = myLength;
            } else {
                anImg.nullify();
            }
            return anImg;
        } else if(aSkippedBytes > 10) {
            //ST_DEBUG_LOG("Extraneous " + (aSkippedBytes - 1) + " padding bytes before section " + aMarker);
        }

        // read the length of the section (including these 2 bytes but excluding marker)
        const int anItemLen = StAlienData::Get16uBE(aData);
        if(anItemLen < 3
        || (aData + anItemLen) > aDataEnd) {
            //ST_DEBUG_LOG("Invalid marker " + aMarker + " in jpeg (item lenght = " + anItemLen
            //           + " from position " + int(aDataEnd - aData - 2) + ')');
            // just ignore probably unknown sections
            continue;
        }

        switch(aMarker) {
            case M_SOF0:
            case M_SOF1:
            case M_SOF2:
            case M_SOF3: {
                if(anItemLen >= 7) {
                    anImg->SizeY = StAlienData::Get16uBE(aData + 2 + 1);
                    anImg->SizeX = StAlienData::Get16uBE(aData + 2 + 3);
                    //ST_DEBUG_LOG("   SOF " + anImg->SizeX + "x" + anImg->SizeY);
                }
                aData += anItemLen;
                break;
            }
            case M_DRI: {
                if(anItemLen == 4) {
                    //const int16_t aNbRestartBlocks = anImg->SizeY = StAlienData::Get16uBE(aData + 2);
                }
                aData += anItemLen;
                break;
            }
            case M_SOS: {
                // here the image data...
                //ST_DEBUG_LOG("Jpeg, SOS at position " + size_t(aData - myBuffer - 1) + " / " + myLength);
                aData += anItemLen;
                break;
            }
            case M_RST0:
            case M_RST1:
            case M_RST2:
            case M_RST3:
            case M_RST4:
            case M_RST5:
            case M_RST6:
            case M_RST7: {
                // aData += aNbRestartBlocks * aMcuSize;
                break;
            }
            case M_JFIF: {
                if(anItemLen >= 16
                && stAreEqual(aData + 2, "JFIF\0", 5)) {
                    myOffsets[Offset_Jfif] = aData - myBuffer - 2;
                    //const int8_t aVerMaj = (int8_t )aData[7];
                    //const int8_t aVerMin = (int8_t )aData[8];
                    const JfifUnitsXY aUnits = (JfifUnitsXY )aData[9];
                    const uint16_t aDensityX = StAlienData::Get16uBE(aData + 10);
                    const uint16_t aDensityY = StAlienData::Get16uBE(aData + 12);
                    //const int8_t  aThumbX   = (int8_t )aData[14];
                    //const int8_t  aThumbY   = (int8_t )aData[15];
                    if(aUnits == JfifUnitsXY_AspectRatio) {
                        anImg->ParX = aDensityX;
                        anImg->ParY = aDensityY;
                    }
                    //ST_DEBUG_LOG("  ## JFIF" + aVerMaj + "." + aVerMin + " u" + (int )aUnits + " " + aDensityX + "x" + aDensityY
                    //           + " thumb " + aThumbX + "x" + aThumbY);
                } else if(stAreEqual(aData + 2, "JFXX\0", 5)) {
                    // JFIF extension
                }

                aData += anItemLen;
                break;
            }
            case M_EXIF:
            case M_APP2: {
                myOffsets[aMarker == M_EXIF ? Offset_Exif : Offset_ExifExtra] = aData - myBuffer - 2;
                // there can be different section using the same marker
                if(stAreEqual(aData + 2, "Exif\0\0", 6)) {
                    //ST_DEBUG_LOG("Exif section...");
                    StHandle<StExifDir> aSubDir = new StExifDir();
                    anImg->Exif.add(aSubDir);
                    if(!aSubDir->parseExif(anImg->Exif, aData + 8, anItemLen - 8)) {
                        //
                    }
                } else if(stAreEqual(aData + 2, "MPF\0", 4)) {
                    // MP Extensions (MPO)
                    StHandle<StExifDir> aSubDir = new StExifDir();
                    aSubDir->Type = StExifDir::DType_MPO;
                    anImg->Exif.add(aSubDir);
                    if(!aSubDir->parseExif(anImg->Exif, aData + 6, anItemLen - 6)) {
                        //
                    }
                } else if(stAreEqual(aData + 2, "http:", 5)) {
                    //ST_DEBUG_LOG("Image cotains XMP section");
                    if(stAreEqual(aData + 2, "http://ns.adobe.com/xap/1.0/", 28)) { // XMP basic namespace
                        myXMP = StString((char* )aData + 31, anItemLen - 31);
                        {
                          // GPano http://ns.google.com/photos/1.0/panorama/ namespace metadata.
                          // Note possible cropped panorama parameters are ignored by sView.
                          if(myXMP.isContains(stCString("<GPano:ProjectionType>equirectangular</GPano:ProjectionType>"))
                          || myXMP.isContains(stCString("GPano:ProjectionType=\"equirectangular\""))) {
                              // Google currently supports only equirectangular format
                              myPanorama = StPanorama_Sphere;
                          } else if(myXMP.isContains(stCString("<GPano:ProjectionType>cubemap</GPano:ProjectionType>"))
                                 || myXMP.isContains(stCString("GPano:ProjectionType=\"cubemap\""))) {
                              // this one doesn't yet exist, but try to support it
                              toDetectCubemap = true;
                          }
                        }
                        myXMP.clear();
                    }
                } else {
                    //ST_DEBUG_LOG("  @@@ APP2 " + StString((char* )aData + 2));
                }
                // skip already read bytes
                aData += anItemLen;
                break;
            }
            case M_APP3: {
                if(anItemLen >= 16
                && stAreEqual(aData + 2, "_JPSJPS_", 8)) {
                    // outdated VRex section
                    myOffsets[Offset_Jps] = aData - myBuffer - 2;
                    //ST_DEBUG_LOG("Jpeg, _JPSJPS_ section (len= )" + anItemLen);
                    //const uint16_t aBlockLen   = StAlienData::Get16uBE(aData + 10);
                    const uint32_t aStereoDesc = StAlienData::Get32uBE(aData + 12);

                    #define SD_LAYOUT_INTERLEAVED 0x00000100
                    #define SD_LAYOUT_SIDEBYSIDE  0x00000200
                    #define SD_LAYOUT_OVERUNDER   0x00000300
                    #define SD_LAYOUT_ANAGLYPH    0x00000400

                    #define SD_HALF_HEIGHT        0x00010000
                    #define SD_HALF_WIDTH         0x00020000
                    #define SD_LEFT_FIELD_FIRST   0x00040000

                    if(aStereoDesc & 0x00000001) {
                        const bool isLeftFirst = (aStereoDesc & SD_LEFT_FIELD_FIRST) != 0;
                        switch(aStereoDesc & 0x0000FF00) {
                            case SD_LAYOUT_INTERLEAVED: myStFormat = StFormat_Rows;            break;
                            case SD_LAYOUT_SIDEBYSIDE:  myStFormat = isLeftFirst
                                                                   ? StFormat_SideBySide_LR
                                                                   : StFormat_SideBySide_RL;   break;
                            case SD_LAYOUT_OVERUNDER:   myStFormat = isLeftFirst
                                                                   ? StFormat_TopBottom_LR
                                                                   : StFormat_TopBottom_RL;    break;
                            case SD_LAYOUT_ANAGLYPH:    myStFormat = StFormat_AnaglyphRedCyan; break;
                            default: break;
                        }
                    } else {
                        myStFormat = StFormat_Mono;
                    }
                    if(anItemLen > 18) {
                        const uint16_t aStringLen = StAlienData::Get16uBE(aData + 16);
                        char* aStrData = (char* )aData + 18;
                        myJpsComment = StString(aStrData, aStringLen);
                    }
                }
                // skip already read bytes
                aData += anItemLen;
                break;
            }
            case M_DQT: {
                myOffsets[Offset_Dqt] = aData - myBuffer - 2;
                aData += anItemLen;
                break;
            }
            case M_APP4:
            case M_APP5:
            case M_APP6:
            case M_APP7:
            case M_APP8:
            case M_APP9:
            case M_APP10:
            case M_APP11:
            case M_APP12:
            case M_APP13:
            case M_APP14:
            case M_APP15:
            case M_DHT: {
                aData += anItemLen;
                break;
            }
            case M_COM: {
                myOffsets[Offset_Comment] = aData - myBuffer - 2;
                if(anItemLen > 2) {
                    myComment = StString((char* )aData + 2, anItemLen - 2);
                }
                //ST_DEBUG_LOG("StJpegParser, comment= '" + myComment + "'");
                aData += anItemLen;
                break;
            }
            default: {
                // carefully skip unknown sections
                //aData += anItemLen;
                break;
            }
        }
    }
}

bool StJpegParser::insertSection(const uint8_t   theMarker,
                                 const uint16_t  theSectLen,
                                 const ptrdiff_t theOffset) {
    const size_t aDiff    = size_t(theSectLen) + 2; // 2 bytes for marker
    const size_t aNewSize = myLength + aDiff;
    if(aNewSize > myBuffSize) {
        myBuffSize = aNewSize + 256;
        stUByte_t* aNewData = stMemAllocAligned<stUByte_t*>(myBuffSize);
        if(aNewData == NULL) {
            return false;
        }
        stMemCpy(aNewData, myBuffer, myLength);
        if(myIsOwnData) {
            stMemFreeAligned(myBuffer);
        }
        myIsOwnData = true;

        // update pointers of image(s) data
        for(StHandle<StJpegParser::Image> anImg = myImages;
            !anImg.isNull(); anImg = anImg->Next) {
            ptrdiff_t anOffset = anImg->Data - myBuffer;
            if(anOffset >= theOffset) {
                anOffset += aDiff;
            }
            anImg->Data = aNewData + anOffset;
            if(!anImg->Thumb.isNull()) {
                anOffset = anImg->Thumb->Data - myBuffer;
                if(anOffset >= theOffset) {
                    anOffset += aDiff;
                }
                anImg->Thumb->Data = aNewData + anOffset;
            }
        }

        myBuffer = aNewData;
    }
    myLength = aNewSize;

    // update offset table
    for(size_t anIter = 0; anIter < OffsetsNb; ++anIter) {
        if(myOffsets[anIter] >= theOffset) {
            myOffsets[anIter] += aDiff;
        }
    }

    // initialize new section
    const size_t aTailSize = myLength - theOffset;
    std::memmove(myBuffer + theOffset + 2 + size_t(theSectLen),
                 myBuffer + theOffset,
                 aTailSize);
    stUByte_t* aData = myBuffer + theOffset;
    aData[0] = 0xFF;
    aData[1] = theMarker;
    StAlienData::Set16uBE(aData + 2, theSectLen);
    return true;
}

bool StJpegParser::setupJps(const StFormat theFormat) {
    if(myBuffer == NULL) {
        return false;
    }

    if(myOffsets[Offset_Jps] == 0) {
        if(myOffsets[Offset_Dqt] == 0) {
            return false;
        }

        // insert section right after DQT
        const StCString THE_APP_DESC = stCString("Written by sView");
        const uint16_t  aDqtLen  = StAlienData::Get16uBE(myBuffer + myOffsets[Offset_Dqt] + 2);
        const ptrdiff_t anOffset = myOffsets[Offset_Dqt] + aDqtLen + 2;
        const uint16_t  aJpsLen  = 16 + 2 + ((uint16_t )THE_APP_DESC.Size + 1);
        if(!insertSection(M_APP3, aJpsLen, anOffset)) {
            return false;
        }

        myOffsets[Offset_Jps] = anOffset;
        stUByte_t* aData = myBuffer + anOffset + 2;
        stMemCpy(aData + 2, "_JPSJPS_", 8);
        StAlienData::Set16uBE(aData + 10, 4);
        StAlienData::Set32uBE(aData + 12, 0);
        StAlienData::Set16uBE(aData + 16, (uint16_t )THE_APP_DESC.Size);
        stMemCpy(aData + 18, THE_APP_DESC.String, THE_APP_DESC.Size + 1);
    } else if(myStFormat == theFormat) {
        return false;
    }

    myStFormat = theFormat;
    uint32_t aStereoDesc = 0x00000001;
    switch(theFormat) {
        case StFormat_SideBySide_LR:
            aStereoDesc |= SD_LAYOUT_SIDEBYSIDE | SD_LEFT_FIELD_FIRST;
            break;
        case StFormat_SideBySide_RL:
            aStereoDesc |= SD_LAYOUT_SIDEBYSIDE;
            break;
        case StFormat_TopBottom_LR:
            aStereoDesc |= SD_LAYOUT_OVERUNDER | SD_LEFT_FIELD_FIRST;
            break;
        case StFormat_TopBottom_RL:
            aStereoDesc |= SD_LAYOUT_OVERUNDER;
            break;
        case StFormat_Rows:
            aStereoDesc |= SD_LAYOUT_INTERLEAVED;
            break;
        case StFormat_AnaglyphRedCyan:
            aStereoDesc |= SD_LAYOUT_ANAGLYPH;
            break;
        case StFormat_Mono:
        default:
            aStereoDesc = 0x00000000;
            break;
    }

    StAlienData::Set32uBE(myBuffer + myOffsets[Offset_Jps] + 2 + 8 + 2 + 2, aStereoDesc);
    return true;
}

StJpegParser::Image::Image()
: Data(NULL),
  Length(0),
  SizeX(0),
  SizeY(0),
  ParX(0),
  ParY(0) {
    //
}

StJpegParser::Image::~Image() {
    //
}

void StJpegParser::fillDictionary(StDictionary& theDict,
                                  const bool    theToShowUnknown) const {
    for(StHandle<StJpegParser::Image> anImg = myImages;
        !anImg.isNull(); anImg = anImg->Next) {
        for(size_t anExifId = 0; anExifId < anImg->Exif.size(); ++anExifId) {
            anImg->Exif[anExifId]->fillDictionary(theDict, theToShowUnknown);
        }
    }
}

StString StJpegParser::Image::getDateTime() const {
    StString aString;
    StExifDir::Query aQuery(StExifDir::DType_General, StExifTags::Image_DateTime);
    if(StExifDir::findEntry(Exif, aQuery)) {
        aQuery.Folder->format(aQuery.Entry, aString);
    }
    return aString;
}

bool StJpegParser::Image::get360PanoMakerNote(bool& theIsStereo) const {
    StExifDir::Query aQuery(StExifDir::DType_General, StExifTags::Image_MakerNote, StExifEntry::FMT_STRING);
    if(!StExifDir::findEntry(Exif, aQuery)) {
        return false;
    } else if(::strncmp((char* )aQuery.Entry.ValuePtr, "360Stereo", 9) == 0) {
        theIsStereo = true;
        return true;
    } else if(::strncmp((char* )aQuery.Entry.ValuePtr, "360Mono", 7) == 0) {
        theIsStereo = false;
        return true;
    }
    return false;
}

bool StJpegParser::Image::getParallax(double& theParallax) const {
    StExifDir::Query aQuery(StExifDir::DType_MakerFuji, StExifTags::Fuji_Parallax);
    if(!StExifDir::findEntry(Exif, aQuery)
    ||  aQuery.Entry.Format != StExifEntry::FMT_SRATIONAL) {
        return false;
    }

    const int32_t aNumerator   = aQuery.Folder->get32s(aQuery.Entry.ValuePtr);
    const int32_t aDenominator = aQuery.Folder->get32s(aQuery.Entry.ValuePtr + 4);
    if(aDenominator != 0) {
        theParallax = double(aNumerator) / double(aDenominator);
        return true;
    }
    return false;
}

StJpegParser::Orient StJpegParser::Image::getOrientation() const {
    StExifDir::Query aQuery(StExifDir::DType_General, StExifTags::Image_Orientation);
    if(!StExifDir::findEntry(Exif, aQuery)
    ||  aQuery.Entry.Format != StExifEntry::FMT_USHORT) {
        return StJpegParser::ORIENT_NORM;
    }

    const int16_t aValue = aQuery.Folder->get16u(aQuery.Entry.ValuePtr);
    return (StJpegParser::Orient )aValue;
}
