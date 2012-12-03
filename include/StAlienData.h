/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
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

        public: //!< static functions

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

        /*private:

    unsigned char* myData; //!< data pointer
    bool    myIsBigEndian; //!< Big-Endian data flag

        public:

    StAlienData(bool theIsBigEndian, unsigned char* theData = NULL)
    : myData(theData),
      myIsBigEndian(theIsBigEndian) {}*/

};

#endif //__StAlienData_h_
