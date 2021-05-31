/**
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StAlienData_h_
#define __StAlienData_h_

#include "stTypes.h"

/**
 * Class to read convert alien data (stored in another bytes order)
 * to native bytes order.
 */
class StAlienData {

        public: //! @name static functions

    /**
     * Retrieve uint8_t.
     */
    static uint8_t Get8u(const stUByte_t* theByte) {
        return theByte[0];
    }

    /**
     * Retrieve int8_t.
     */
    static int8_t Get8s(const stUByte_t* theByte) {
        return ((char* )theByte)[0];
    }

    /**
     * Retrieve int16_t.
     */
    static int16_t Get16s(const stUByte_t* theShort,
                          const bool       theIsBE) {
        return theIsBE ? Get16sBE(theShort) : Get16sLE(theShort);
    }

    /**
     * Retrieve uint16_t.
     */
    static uint16_t Get16u(const stUByte_t* theShort,
                           const bool       theIsBE) {
        return theIsBE ? Get16uBE(theShort) : Get16uLE(theShort);
    }

    /**
     * Store uint16_t.
     */
    static void Set16u(stUByte_t*     thePtr,
                       const uint16_t theValue,
                       const bool     theIsBE) {
        theIsBE ? Set16uBE(thePtr, theValue) : Set16uLE(thePtr, theValue);
    }

    /**
     * Retrieve int16_t stored in Little-Endian order.
     */
    static int16_t Get16sLE(const stUByte_t* theShort) {
        return (((char* )theShort)[1] << 8) | theShort[0];
    }

    /**
     * Retrieve int16_t stored in Big-Endian order.
     */
    static int16_t Get16sBE(const stUByte_t* theShort) {
        return (((char* )theShort)[0] << 8) | theShort[1];
    }

    /**
     * Retrieve uint16_t stored in Little-Endian order.
     */
    static uint16_t Get16uLE(const stUByte_t* theShort) {
        return (theShort[1] << 8) | theShort[0];
    }

    /**
     * Retrieve uint16_t stored in Big-Endian order.
     */
    static uint16_t Get16uBE(const stUByte_t* theShort) {
        return (theShort[0] << 8) | theShort[1];
    }

    /**
     * Store uint16_t in Little-Endian order.
     */
    static void Set16uLE(stUByte_t*     thePtr,
                         const uint16_t theValue) {
        thePtr[0] = (theValue & 0x00FF);
        thePtr[1] = (theValue & 0xFF00) >> 8;
    }

    /**
     * Store uint16_t in Big-Endian order.
     */
    static void Set16uBE(stUByte_t*     thePtr,
                         const uint16_t theValue) {
        thePtr[0] = (theValue & 0xFF00) >> 8;
        thePtr[1] = (theValue & 0x00FF);
    }

    /**
     * Retrieve int32_t.
     */
    static int32_t Get32s(const stUByte_t* theLong,
                          const bool       theIsBE) {
        return theIsBE ? Get32sBE(theLong) : Get32sLE(theLong);
    }

    /**
     * Retrieve int32_t stored in Little-Endian order.
     */
    static int32_t Get32sLE(const stUByte_t* theLong) {
        return (((char*      )theLong)[3] << 24) | (((stUByte_t* )theLong)[2] << 16)
             | (((stUByte_t* )theLong)[1] << 8 ) | (((stUByte_t* )theLong)[0] << 0 );
    }

    /**
     * Retrieve int32_t stored in Big-Endian order.
     */
    static int32_t Get32sBE(const stUByte_t* theLong) {
        return (((char*      )theLong)[0] << 24) | (((stUByte_t* )theLong)[1] << 16)
             | (((stUByte_t* )theLong)[2] << 8 ) | (((stUByte_t* )theLong)[3] << 0 );
    }

    /**
     * Retrieve uint32_t.
     */
    static uint32_t Get32u(const stUByte_t* theLong,
                           const bool       theIsBE) {
        return theIsBE ? Get32uBE(theLong) : Get32uLE(theLong);
    }

    /**
     * Store uint32_t.
     */
    static void Set32u(stUByte_t*     thePtr,
                       const uint32_t theValue,
                       const bool     theIsBE) {
        theIsBE ? Set32uBE(thePtr, theValue) : Set32uLE(thePtr, theValue);
    }

    /**
     * Retrieve uint32_t stored in Little-Endian order.
     */
    static uint32_t Get32uLE(const stUByte_t* theLong) {
        return (((char*      )theLong)[3] << 24) | (((stUByte_t* )theLong)[2] << 16)
             | (((stUByte_t* )theLong)[1] << 8 ) | (((stUByte_t* )theLong)[0] << 0 );
    }

    /**
     * Retrieve uint32_t stored in Big-Endian order.
     */
    static uint32_t Get32uBE(const stUByte_t* theLong) {
        return (((char*      )theLong)[0] << 24) | (((stUByte_t* )theLong)[1] << 16)
             | (((stUByte_t* )theLong)[2] << 8 ) | (((stUByte_t* )theLong)[3] << 0 );
    }

    /**
     * Store uint32_t in Little-Endian order.
     */
    static void Set32uLE(stUByte_t*     thePtr,
                         const uint32_t theValue) {
        thePtr[0] = stUByte_t((theValue & 0x000000FF));
        thePtr[1] = stUByte_t((theValue & 0x0000FF00) >> 8);
        thePtr[2] = stUByte_t((theValue & 0x00FF0000) >> 16);
        thePtr[3] = stUByte_t((theValue & 0xFF000000) >> 24);
    }

    /**
     * Store uint32_t in Big-Endian order.
     */
    static void Set32uBE(stUByte_t*     thePtr,
                         const uint32_t theValue) {
        thePtr[0] = stUByte_t((theValue & 0xFF000000) >> 24);
        thePtr[1] = stUByte_t((theValue & 0x00FF0000) >> 16);
        thePtr[2] = stUByte_t((theValue & 0x0000FF00) >> 8);
        thePtr[3] = stUByte_t((theValue & 0x000000FF));
    }

};

#endif // __StAlienData_h_
