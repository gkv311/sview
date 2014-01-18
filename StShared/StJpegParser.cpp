/**
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <cstdio> // forward-declared to work-around broken MinGW headers

#include <StImage/StJpegParser.h>

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

StJpegParser::StJpegParser()
: myImages(NULL),
  myData(NULL),
  myLength(0),
  myStFormat(ST_V_SRC_AUTODETECT) {
    //
}

StJpegParser::~StJpegParser() {
    reset();
}

void StJpegParser::reset() {
    // destroy all images
    myImages.nullify();
    myComment.clear();
    myStFormat = ST_V_SRC_AUTODETECT;

    // destroy cached data
    stMemFreeAligned(myData);
    myData = NULL;
    myLength = 0;
}

bool StJpegParser::read(const StString& theFileName) {
    // clean up old data
    reset();

#if defined(_WIN32)
    FILE* aFileHandle = _wfopen(theFileName.toUtfWide().toCString(), L"rb");
#else
    FILE* aFileHandle =   fopen(theFileName.toCString(),              "rb");
#endif

    if(aFileHandle == NULL) {
        return false;
    }

    // determine length of file
    fseek(aFileHandle, 0, SEEK_END);
    long aFileLen = ftell(aFileHandle);
    if(aFileLen <= 0L) {
        fclose(aFileHandle);
        return false;
    }

    myLength = size_t(aFileLen);
    fseek(aFileHandle, 0, SEEK_SET);

    myData = stMemAllocAligned<unsigned char*>(myLength);
    size_t aCountRead = fread(myData, 1, myLength, aFileHandle);
    (void )aCountRead;

    fclose(aFileHandle);

    // parse the structure
    return parse();
}

bool StJpegParser::parse() {
    if(myData == NULL) {
        return false;
    }

    myImages = parseImage(myData);
    if(myImages.isNull()) {
        return false;
    }

    // continue reading the file (MPO may contains more than 1 image)
    for(StHandle<StJpegParser::Image> anImg = myImages;
        !anImg.isNull(); anImg = anImg->myNext) {
        //ST_DEBUG_LOG("curr= " + size_t(anImg->myData) + "; next= " + size_t(anImg->myData + anImg->myLength));
        anImg->myNext = parseImage(anImg->myData + anImg->myLength);
    }

    return true;
}

StHandle<StJpegParser::Image> StJpegParser::parseImage(unsigned char* theDataStart) {
    unsigned char* aData    = theDataStart;
    unsigned char* aDataEnd = myData + myLength; // common data limit

    // check out of bounds
    if(theDataStart == NULL
    || (aData + 2) > aDataEnd) {
        return StHandle<StJpegParser::Image>();
    }

    // check the jpeg identifier
    if(aData[0] != 0xFF || aData[1] != M_SOI) {
        ST_DEBUG_LOG("Not a JPEG file");
        return StHandle<StJpegParser::Image>();
    }
    aData += 2; // skip already read bytes

    // parse the data
    StHandle<StJpegParser::Image> anImg = new StJpegParser::Image();
    anImg->myData = aData - 2;

    int           aSubImg = 0; // subimages counters
    unsigned char aMarker = 0;
    for(;;) {
        // search for the next marker in the file
        ++aData; // one byte forward
        size_t aSkippedBytes  = 0;
        for(; aData < aDataEnd; ++aSkippedBytes, ++aData) {
            aMarker = aData[0];
            if(aData[-1] == 0xFF && aMarker != 0xFF) {
                ++aData; // skip already read byte
                break;
            }
        }

        if(aMarker == M_EOI) {
            // End Of Image detected
            if(aSubImg-- > 0) {
                ++aData;
                continue;
            }
            // skip all zeros before next image in MPO...
            for(; (aData < aDataEnd) && (aData[0] == 0x00); ++aData) {
                //ST_DEBUG_LOG("Jpeg, skipped zero");
            }
            --aData;
            anImg->myLength = size_t(aData - theDataStart + 1);
            //ST_DEBUG_LOG("Jpeg, EOI at position " + size_t(aData - myData - 1) + " / " + myLength);
            return anImg;
        }

        if((aData + 2) >= aDataEnd) {
            ST_DEBUG_LOG("Corrupt jpeg file or error in parser");
            ///anImg.nullify();
            if(myImages.isNull()) {
                anImg->myData   = myData;
                anImg->myLength = myLength;
            }
            return anImg;
        } else if(aSkippedBytes > 10) {
            //ST_DEBUG_LOG("Extraneous " + (aSkippedBytes - 1) + " padding bytes before section " + aMarker);
        }

        //ST_DEBUG_LOG("Jpeg marker " + aMarker + " at position " + size_t(aData - myData - 1));

        // read the length of the section.
        const int aLenH = aData[0];
        const int aLenL = aData[1];
        const int anItemLen = (aLenH << 8) | aLenL;
        if(anItemLen < 2
        || (aData + anItemLen + 2) > aDataEnd) {
            //ST_DEBUG_LOG("Invalid marker " + aMarker + " in jpeg (item lenght = " + anItemLen
            //           + " from position " + int(aDataEnd - aData - 2) + ')');
            // just ignore probably unknown sections
            aData += 2;
            continue;
        }

        switch(aMarker) {
            case M_SOI: {
                // here the subimage (thumbnail)...
                //ST_DEBUG_LOG("Jpeg, SOI at position " + size_t(aData - myData - 1) + " / " + myLength);
                aData += 2;
                ++aSubImg;
                break;
            }
            case M_SOS: {
                // here the image data...
                //ST_DEBUG_LOG("Jpeg, SOS at position " + size_t(aData - myData - 1) + " / " + myLength);
                aData += anItemLen + 2;
                break;
            }
            case M_JFIF: {
                aData += anItemLen + 2;
                break;
            }
            case M_EXIF:
            case M_APP2: {
                // there can be different section using the same marker
                if(stAreEqual(&aData[2], "Exif", 4)) {
                    //ST_DEBUG_LOG("Exif section...");
                    StHandle<StExifDir> aSubDir = new StExifDir(true);
                    if(aSubDir->parseExif(aData, anItemLen)) {
                        anImg->myExif.add(aSubDir);
                    }
                } else if(stAreEqual(&aData[2], "http:", 5)) {
                    //ST_DEBUG_LOG("Image cotains XMP section");
                }
                // skip already read bytes
                aData += anItemLen + 2;
                break;
            }
            case M_APP3: {
                if(anItemLen >= 14
                && stAreEqual(&aData[2], "_JPSJPS_", 8)) {
                    // outdated VRex section
                    ST_DEBUG_LOG("Jpeg, _JPSJPS_ section");
                    //const int16_t aBlockLen   = StAlienData::Get16sBE(&aData[2 + 8]);
                    const int32_t aStereoDesc = StAlienData::Get32sBE(&aData[2 + 8 + 2]);

                    #define SD_LAYOUT_INTERLEAVED 0x00000100
                    #define SD_LAYOUT_SIDEBYSIDE  0x00000200
                    #define SD_LAYOUT_OVERUNDER   0x00000300
                    #define SD_LAYOUT_ANAGLYPH    0x00000400

                    #define SD_HALF_HEIGHT        0x00010000
                    #define SD_HALF_WIDTH         0x00020000
                    #define SD_LEFT_FIELD_FIRST   0x00040000

                    if(aStereoDesc & 0x00000001) {
                        const bool isLeftFirst = aStereoDesc & SD_LEFT_FIELD_FIRST;
                        switch(aStereoDesc & 0x0000FF00) {
                            case SD_LAYOUT_INTERLEAVED: myStFormat = ST_V_SRC_ROW_INTERLACE;     break;
                            case SD_LAYOUT_SIDEBYSIDE:  myStFormat = isLeftFirst
                                                                   ? ST_V_SRC_PARALLEL_PAIR
                                                                   : ST_V_SRC_SIDE_BY_SIDE;      break;
                            case SD_LAYOUT_OVERUNDER:   myStFormat = isLeftFirst
                                                                   ? ST_V_SRC_OVER_UNDER_LR
                                                                   : ST_V_SRC_OVER_UNDER_RL;     break;
                            case SD_LAYOUT_ANAGLYPH:    myStFormat = ST_V_SRC_ANAGLYPH_RED_CYAN; break;
                            default: break;
                        }
                    }
                }
                // skip already read bytes
                aData += anItemLen + 2;
                break;
            }
            case M_COM: {
                myComment = StString((char* )aData + 2, anItemLen);
                ST_DEBUG_LOG("StJpegParser, comment= '" + myComment + "'");
                aData += anItemLen + 2;
                break;
            }
            default: {
                // carefully skip unknown sections
                aData += 2;
                break;
            }
        }

    }
}

StJpegParser::Image::Image()
: myData(NULL),
  myLength(0) {
    //
}

StJpegParser::Image::~Image() {
    //
}

bool StJpegParser::Image::getParallax(double& theParallax) const {
    StExifEntry anEntry;
    bool isBigEndian = false;
    for(size_t anExifId = 0; anExifId < myExif.size(); ++anExifId) {
        const StHandle<StExifDir>& aDir = myExif[anExifId];
        if(aDir.isNull()) {
            // should never happens
            continue;
        }
        if(aDir->getCameraMaker() == StString("FUJIFILM")) {
            anEntry.myTag = 0xB211;
            if(aDir->findEntry(true, anEntry, isBigEndian)) {
                if(anEntry.myFormat == StExifEntry::FMT_SRATIONAL) {
                    int32_t aNumerator = isBigEndian ? StAlienData::Get32sBE(anEntry.myValuePtr)
                                                     : StAlienData::Get32sLE(anEntry.myValuePtr);
                    int32_t aDenominator = isBigEndian ? StAlienData::Get32sBE(anEntry.myValuePtr + 4)
                                                       : StAlienData::Get32sLE(anEntry.myValuePtr + 4);
                    if(aDenominator != 0) {
                        theParallax = double(aNumerator) / double(aDenominator);
                        //ST_DEBUG_LOG("Parallax found(" + aNumerator + " / " + aDenominator + ")= " + theParallax);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

StJpegParser::Orient StJpegParser::Image::getOrientation() const {
    StExifEntry anEntry;
    anEntry.myTag = 0x112;
    bool isBigEndian = false;
    for(size_t anExifId = 0; anExifId < myExif.size(); ++anExifId) {
        const StHandle<StExifDir>& aDir = myExif[anExifId];
        if(!aDir.isNull()
        && aDir->findEntry(false, anEntry, isBigEndian)
        && anEntry.myFormat == StExifEntry::FMT_USHORT) {
            int16_t aValue = isBigEndian ? StAlienData::Get16uBE(anEntry.myValuePtr)
                                         : StAlienData::Get16uLE(anEntry.myValuePtr);
            return (StJpegParser::Orient )aValue;
        }
    }
    return StJpegParser::ORIENT_NORM;
}
