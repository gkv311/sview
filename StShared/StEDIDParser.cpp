/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StCore/StEDIDParser.h>
#include <StAlienData.h>

#ifndef _MSC_VER
    #include <stdio.h>
#endif

enum {
    ST_EDID1_DESC1 = 54,
    ST_EDID1_DESC2 = 72,
    ST_EDID1_DESC3 = 90,
    ST_EDID1_DESC4 = 108,
    ST_EDID1_DESC1_PIXEL_CLOCK      = ST_EDID1_DESC1 + 0,
    ST_EDID1_DESC1_SIGNAL_INTERFACE = ST_EDID1_DESC1 + 17,
};

StEDIDParser::StEDIDParser()
: myData(NULL) {
    //
}

StEDIDParser::StEDIDParser(const stUByte_t* theData)
: myData(NULL) {
    init(theData);
}

StEDIDParser::StEDIDParser(const StEDIDParser& theCopy)
: myData(NULL) {
    init(theCopy.myData);
}

const StEDIDParser& StEDIDParser::operator=(const StEDIDParser& theCopy) {
    init(theCopy.myData);
    return *this;
}

void StEDIDParser::clear() {
    if(myData != NULL) {
        delete[] myData;
        myData = NULL;
    }
}

StEDIDParser::~StEDIDParser() {
    clear();
}

void StEDIDParser::init(const stUByte_t* theData) {
    clear();
    if(theData != NULL) {
        myData = new stUByte_t[128];
        stMemCpy(myData, theData, 128);
    }
}

bool StEDIDParser::isFirstVersion() const {
    static const stUByte_t EDID_V1_HEADER[] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
    return stAreEqual(myData, EDID_V1_HEADER, sizeof(EDID_V1_HEADER));
}

bool StEDIDParser::isValid() const {
    if(myData == NULL) {
        return false;
    }
    stUByte_t aCheckSumm = 0;
    for(size_t aByteId = 0; aByteId < 128; ++aByteId) {
        aCheckSumm += myData[aByteId];
    }
    return (aCheckSumm == 0) && isFirstVersion();
}

void StEDIDParser::validate() {
    stUByte_t aCheckSumm = 0;
    for(size_t aByteId = 0; aByteId < 127; ++aByteId) {
        aCheckSumm += myData[aByteId];
    }
    myData[127] = -aCheckSumm;
}

unsigned int StEDIDParser::getVersion() const {
    return (unsigned int )myData[18];
}

unsigned int StEDIDParser::getRevision() const {
    return (unsigned int )myData[19];
}

unsigned int StEDIDParser::getYear() const {
    return 1990U + myData[17];
}

unsigned int StEDIDParser::getWeek() const {
    return (unsigned int )myData[16];
}

double StEDIDParser::getGamma() const {
    return 1.0 + 0.001 * myData[23];
}

StString StEDIDParser::getName() const {
    stUByte_t const* aDataPtr = myData + 54;
    for(size_t aBlockId = 0; aBlockId < 4; ++aBlockId, aDataPtr += 18) {
        if(aDataPtr[0] == 0x00 && aDataPtr[1] == 0x00 &&
           aDataPtr[2] == 0x00 && aDataPtr[3] == 0xFC) {
            char aName[13];
            aDataPtr += 5;
            for(size_t aCharId = 0; aCharId < 13; ++aCharId, ++aDataPtr) {
                if(*aDataPtr == 0xA) {
                    aName[aCharId] = '\0';
                    return StString(aName);
                }
                aName[aCharId] = *aDataPtr;
            }
            aName[12] = '\0';
            return StString(aName);
        }
    }
    return StString();
}

/**
 * Auxiliary function to extract bits from number.
 */
inline unsigned char getBits(const unsigned char theByte,
                             const unsigned char theBitFrom,
                             const unsigned char theBitsNb) {
    return (theByte >> theBitFrom) & ((1 << theBitsNb) - 1);
}

StString StEDIDParser::getPnPId() const {
    if(myData == NULL) {
        return "AAA0000";
    }

    char pnpid[256];
#if defined(_MSC_VER)
    sprintf_s(
#else
    snprintf(
#endif
    pnpid, 8, "%c%c%c%02X%02X",
    // Manufacturer ID. These IDs are assigned by Microsoft.
    getBits(myData[8], 2, 5) + 'A' - 1,
   (getBits(myData[8], 0, 2) << 3) + getBits(myData[9], 5, 3) + 'A' - 1,
    getBits(myData[9], 0, 5) + 'A' - 1,
    // Product ID Code (stored as LSB first). Assigned by manufacturer.
      myData[11], myData[10]);
    return StString(pnpid);
}

inline unsigned char hex2number(const unsigned char theLetter) {
    return (theLetter >= 'A') ? (theLetter - 'A' + 10) : (theLetter - '0');
}

void StEDIDParser::setPnPId(const StString& thePnPIdString) {
    if(thePnPIdString.getLength() != 7) {
        return;
    }
    const unsigned char* aStr = (const unsigned char* )thePnPIdString.toCString();

    unsigned char aBits15_10 = aStr[0] - 'A' + 1;
    unsigned char aBits09_05 = aStr[1] - 'A' + 1;
    unsigned char aBits04_00 = aStr[2] - 'A' + 1;

    myData[8]  = getBits(aBits09_05, 3, 2) + (getBits(aBits15_10, 0, 5) << 2);
    myData[9]  = getBits(aBits04_00, 0, 5) + (getBits(aBits09_05, 0, 3) << 5);
    myData[10] = hex2number(aStr[5]) * 16 + hex2number(aStr[6]);
    myData[11] = hex2number(aStr[3]) * 16 + hex2number(aStr[4]);

    validate();
}

StEDIDParser::stEdid1Stereo_t StEDIDParser::getStereoFlag() const {
    uint16_t aPixelClock = StAlienData::Get16uLE(&myData[ST_EDID1_DESC1_PIXEL_CLOCK]);
    if(aPixelClock == 0) {
        return StEDIDParser::STEREO_NO;
    }
    stUByte_t aSignalType = myData[ST_EDID1_DESC1_SIGNAL_INTERFACE];
    bool aBit6 = (aSignalType & (1 << 6)) != 0;
    bool aBit5 = (aSignalType & (1 << 5)) != 0;
    bool aBit0 = (aSignalType & (1 << 0)) != 0;
    if(!aBit5 && !aBit6) {
        return StEDIDParser::STEREO_NO;
    } else if(!aBit6 &&  aBit5 && !aBit0) {
        return StEDIDParser::STEREO_PAGEFLIP_R;
    } else if( aBit6 && !aBit5 &&  aBit0) {
        return StEDIDParser::STEREO_PAGEFLIP_L;
    } else if(!aBit6 &&  aBit5 &&  aBit0) {
        return StEDIDParser::STEREO_INTERLEAVED_2WAY_R;
    } else if( aBit6 && !aBit5 &&  aBit0) {
        return StEDIDParser::STEREO_INTERLEAVED_2WAY_L;
    } else if( aBit6 &&  aBit5 && !aBit0) {
        return StEDIDParser::STEREO_INTERLEAVED_4WAY;
    } else if( aBit6 &&  aBit5 &&  aBit0) {
        return StEDIDParser::STEREO_SIDEBYSIDE;
    }
    return StEDIDParser::STEREO_NO;
}

StString StEDIDParser::getStereoString() const {
    switch(getStereoFlag()) {
        case StEDIDParser::STEREO_PAGEFLIP_R:
        case StEDIDParser::STEREO_PAGEFLIP_L:
            return "Pageflip";
        case StEDIDParser::STEREO_INTERLEAVED_2WAY_R:
        case StEDIDParser::STEREO_INTERLEAVED_2WAY_L:
            return "Interlace";
        case StEDIDParser::STEREO_INTERLEAVED_4WAY:
            return "Interleaved 4-way";
        case StEDIDParser::STEREO_NO:
        default:
            return "No stereo";
    }
}
