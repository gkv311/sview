/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
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
    M_SOI   = 0xD8, // Start Of Image (beginning of datastream)
    M_EOI   = 0xD9, // End Of Image (end of datastream)
    M_SOS   = 0xDA, // Start Of Scan (begins compressed data)
    M_JFIF  = 0xE0, // Jfif marker
    M_EXIF  = 0xE1, // Exif attribute information (APP1)
    M_APP2  = 0xE2, // Exif extended data
    M_COM   = 0xFE, // COMment
    M_DQT   = 0xDB, // Quantization table definition
    M_DHT   = 0xC4, // Huffman table definition
    M_DRI   = 0xDD, // Restart Interoperability definition
    M_IPTC  = 0xED, // IPTC marker
};

StJpegParser::StJpegParser()
: myImages(NULL),
  myData(NULL),
  myLength(0) {
    //
}

StJpegParser::~StJpegParser() {
    reset();
}

void StJpegParser::reset() {
    // destroy all images
    myImages.nullify();

    // destroy cached data
    if(myData != NULL) {
        delete[] myData;
        myData = NULL;
    }
    myLength = 0;
}

bool StJpegParser::read(const StString& theFileName) {
    // clean up old data
    reset();

#if (defined(_WIN32) || defined(__WIN32__))
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

    myData = new unsigned char[myLength];
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
    StHandle<StJpegParser::Image> anImg;

    // check out of bounds
    if(theDataStart == NULL || (aData + 2) > aDataEnd) {
        return anImg;
    }

    // check the jpeg identifier
    if(aData[0] != 0xFF || aData[1] != M_SOI) {
        ST_DEBUG_LOG("Not a JPEG file");
        return anImg;
    }
    aData += 2; // skip already read bytes

    // parse the data
    anImg = new StJpegParser::Image();
    anImg->myData = aData - 2;

    size_t aSubImg = 0; // subimages counters

    size_t aSkippedBytes  = 0;
    unsigned char aMarker = 0;
    for(;;) {
        // search for the next marker in the file
        ++aData; // one byte forward
        for(aSkippedBytes = 0; aData < aDataEnd; ++aSkippedBytes, ++aData) {
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
        int aLenH = aData[0];
        int aLenL = aData[1];
        int anItemLen = (aLenH << 8) | aLenL;
        if(anItemLen < 2 || (aData + anItemLen + 2) > aDataEnd) {
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
            case M_EXIF:
            case M_APP2: {
                // there can be different section using the same marker.
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
            default: {
                // carefully skip unknown sections
                aData += 2;
                break;
            }
        }

    }
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
