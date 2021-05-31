/**
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StExifEntry_h_
#define __StExifEntry_h_

#include <stTypes.h>

/**
 * One entry in the Exif directory (StExifDir).
 */
struct StExifEntry {

        public:

    unsigned char* ValuePtr;   //!< pointer to the entry value(s)
    uint16_t       Tag;        //!< entry tag
    uint16_t       Format;     //!< data type
    uint32_t       Components; //!< components number

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
        return size_t(Components) * BYTES_PER_FORMAT[Format];
    }

        public: //! @name comparators

    bool operator==(const StExifEntry& theCompare) const { return (ValuePtr == theCompare.ValuePtr); }
    bool operator!=(const StExifEntry& theCompare) const { return (ValuePtr != theCompare.ValuePtr); }
    bool operator> (const StExifEntry& theCompare) const { return (ValuePtr >  theCompare.ValuePtr); }
    bool operator< (const StExifEntry& theCompare) const { return (ValuePtr <  theCompare.ValuePtr); }
    bool operator>=(const StExifEntry& theCompare) const { return (ValuePtr >= theCompare.ValuePtr); }
    bool operator<=(const StExifEntry& theCompare) const { return (ValuePtr <= theCompare.ValuePtr); }

};

#endif // __StExifEntry_h_
