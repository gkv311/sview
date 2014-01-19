/**
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
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
     * Retrieve uint16_t.
     */
    static uint16_t Get16u(const unsigned char* theShort,
                           const bool           theIsBE) {
        return theIsBE ? Get16uBE(theShort) : Get16uLE(theShort);
    }

    /**
     * Store uint16_t.
     */
    static void Set16u(unsigned char* thePtr,
                       const uint16_t theValue,
                       const bool     theIsBE) {
        theIsBE ? Set16uBE(thePtr, theValue) : Set16uLE(thePtr, theValue);
    }

    /**
     * Retrieve uint16_t stored in Little-Endian order.
     */
    static uint16_t Get16uLE(const unsigned char* theShort) {
        return (theShort[1] << 8) | theShort[0];
    }

    /**
     * Retrieve uint16_t stored in Big-Endian order.
     */
    static uint16_t Get16uBE(const unsigned char* theShort) {
        return (theShort[0] << 8) | theShort[1];
    }

    /**
     * Store uint16_t in Little-Endian order.
     */
    static void Set16uLE(unsigned char* thePtr,
                         const uint16_t theValue) {
        thePtr[0] = (theValue & 0x00FF);
        thePtr[1] = (theValue & 0xFF00) >> 8;
    }

    /**
     * Store uint16_t in Big-Endian order.
     */
    static void Set16uBE(unsigned char* thePtr,
                         const uint16_t theValue) {
        thePtr[0] = (theValue & 0xFF00) >> 8;
        thePtr[1] = (theValue & 0x00FF);
    }

    /**
     * Retrieve int32_t.
     */
    static int32_t Get32s(const unsigned char* theLong,
                          const bool           theIsBE) {
        return theIsBE ? Get32sBE(theLong) : Get32sLE(theLong);
    }

    /**
     * Retrieve int32_t stored in Little-Endian order.
     */
    static int32_t Get32sLE(const unsigned char* theLong) {
        return (((char*          )theLong)[3] << 24) | (((unsigned char* )theLong)[2] << 16)
             | (((unsigned char* )theLong)[1] << 8 ) | (((unsigned char* )theLong)[0] << 0 );
    }

    /**
     * Retrieve int32_t stored in Big-Endian order.
     */
    static int32_t Get32sBE(const unsigned char* theLong) {
        return (((char*          )theLong)[0] << 24) | (((unsigned char* )theLong)[1] << 16)
             | (((unsigned char* )theLong)[2] << 8 ) | (((unsigned char* )theLong)[3] << 0 );
    }

    /**
     * Retrieve uint32_t.
     */
    static uint32_t Get32u(const unsigned char* theLong,
                           const bool           theIsBE) {
        return theIsBE ? Get32uBE(theLong) : Get32uLE(theLong);
    }

    /**
     * Store uint32_t.
     */
    static void Set32u(unsigned char* thePtr,
                       const uint32_t theValue,
                       const bool     theIsBE) {
        theIsBE ? Set32uBE(thePtr, theValue) : Set32uLE(thePtr, theValue);
    }

    /**
     * Retrieve uint32_t stored in Little-Endian order.
     */
    static uint32_t Get32uLE(const unsigned char* theLong) {
        return (((char*          )theLong)[3] << 24) | (((unsigned char* )theLong)[2] << 16)
             | (((unsigned char* )theLong)[1] << 8 ) | (((unsigned char* )theLong)[0] << 0 );
    }

    /**
     * Retrieve uint32_t stored in Big-Endian order.
     */
    static uint32_t Get32uBE(const unsigned char* theLong) {
        return (((char*          )theLong)[0] << 24) | (((unsigned char* )theLong)[1] << 16)
             | (((unsigned char* )theLong)[2] << 8 ) | (((unsigned char* )theLong)[3] << 0 );
    }

    /**
     * Store uint32_t in Little-Endian order.
     */
    static void Set32uLE(unsigned char* thePtr,
                         const uint32_t theValue) {
        thePtr[0] = (theValue & 0x000000FF);
        thePtr[1] = (theValue & 0x0000FF00) >> 8;
        thePtr[2] = (theValue & 0x00FF0000) >> 16;
        thePtr[3] = (theValue & 0xFF000000) >> 24;
    }

    /**
     * Store uint32_t in Big-Endian order.
     */
    static void Set32uBE(unsigned char* thePtr,
                         const uint32_t theValue) {
        thePtr[0] = (theValue & 0xFF000000) >> 24;
        thePtr[1] = (theValue & 0x00FF0000) >> 16;
        thePtr[2] = (theValue & 0x0000FF00) >> 8;
        thePtr[3] = (theValue & 0x000000FF);
    }

        /*private:

    unsigned char* myData; //!< data pointer
    bool    myIsBigEndian; //!< Big-Endian data flag

        public:

    StAlienData(bool theIsBigEndian, unsigned char* theData = NULL)
    : myData(theData),
      myIsBigEndian(theIsBigEndian) {}*/

};

#endif // __StAlienData_h_
