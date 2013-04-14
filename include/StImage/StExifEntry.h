/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StExifEntry_h_
#define __StExifEntry_h_

#include <stTypes.h>

/**
 * One entry in the Exif directory (StExifDir).
 */
struct StExifEntry {

        public:

    unsigned char* myValuePtr;   //!< pointer to the entry value(s)
    uint16_t       myTag;        //!< entry tag
    uint16_t       myFormat;     //!< data type
    uint32_t       myComponents; //!< components number

        public:

    /**
     * List of acceptable data types.
     */
    enum {
        FMT_BYTE      =  1, //!< unsigned byte
        FMT_STRING    =  2, //!< 8-bit byte containing one 7-bit ASCII code; the final byte is terminated with NULL
        FMT_USHORT    =  3, //!< 16-bit unsigned integer type
        FMT_ULONG     =  4, //!< 32-bit unsigned integer type
        FMT_URATIONAL =  5, //!< rational type represented with 2x FMT_ULONG values
        FMT_SBYTE     =  6, //!< signed byte
        FMT_UNDEFINED =  7, //!< undefined type
        FMT_SSHORT    =  8, //!< 16-bit signed integer type
        FMT_SLONG     =  9, //!< 32-bit signed integer type
        FMT_SRATIONAL = 10, //!< rational type represented with 2x FMT_SLONG values
        FMT_SINGLE    = 11,
        FMT_DOUBLE    = 12,
        NUM_FORMATS   = 12, //!< last acceptable format id
    };

    /**
     * Simple array for size of each format in bytes (per one element).
     */
    static const size_t BYTES_PER_FORMAT[];

        public:

    /**
     * Returns the bytes number for entry values data.
     */
    inline size_t getBytes() const {
        return size_t(myComponents) * BYTES_PER_FORMAT[myFormat];
    }

        public: //!< comparators

    bool operator==(const StExifEntry& theCompare) const {
        return (myValuePtr == theCompare.myValuePtr);
    }

    bool operator!=(const StExifEntry& theCompare) const {
        return (myValuePtr != theCompare.myValuePtr);
    }

    bool operator>(const StExifEntry& theCompare) const {
        return (myValuePtr > theCompare.myValuePtr);
    }

    bool operator<(const StExifEntry& theCompare) const {
        return (myValuePtr < theCompare.myValuePtr);
    }

    bool operator>=(const StExifEntry& theCompare) const {
        return (myValuePtr >= theCompare.myValuePtr);
    }

    bool operator<=(const StExifEntry& theCompare) const {
        return (myValuePtr <= theCompare.myValuePtr);
    }

};

#endif //__StExifEntry_h_
