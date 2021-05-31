/**
 * Copyright © 2008 Daniel Remenak <dtremenak@users.sourceforge.net>
 * Copyright © 2001-2004 Unicode, Inc.
 *
 * Disclaimer
 *
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 *
 * Limitations on Rights to Redistribute This Code
 *
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 *
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StUtfIterator_inl_
#define __StUtfIterator_inl_

/**
 * The first character in a UTF-8 sequence indicates how many bytes
 * to read (among other things).
 */
template<typename Type>
const unsigned char StUtfIterator<Type>::UTF8_BYTES_MINUS_ONE[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/**
 * Magic values subtracted from a buffer value during UTF-8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence.
 */
template<typename Type>
const unsigned long StUtfIterator<Type>::offsetsFromUTF8[6] = {
    0x00000000UL, 0x00003080UL, 0x000E2080UL,
    0x03C82080UL, 0xFA082080UL, 0x82082080UL
};

/**
 * The first character in a UTF-8 sequence indicates how many bytes to read.
 */
template<typename Type>
const unsigned char StUtfIterator<Type>::UTF8_FIRST_BYTE_MARK[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/**
 * Get a UTF-8 character; leave the tracking pointer at the start of the next character.
 * Not protected against invalid UTF-8.
 */
template<typename Type>
inline void StUtfIterator<Type>::readUTF8() {
    // unsigned arithmetic used
    stUtf8u_t* aPos = (stUtf8u_t* )myPosNext;
    const unsigned char aBytesToRead = UTF8_BYTES_MINUS_ONE[*aPos];
    myCharUtf32 = 0;
    switch(aBytesToRead) {
        case 5: myCharUtf32 += *aPos++; myCharUtf32 <<= 6; // remember, illegal UTF-8
        case 4: myCharUtf32 += *aPos++; myCharUtf32 <<= 6; // remember, illegal UTF-8
        case 3: myCharUtf32 += *aPos++; myCharUtf32 <<= 6;
        case 2: myCharUtf32 += *aPos++; myCharUtf32 <<= 6;
        case 1: myCharUtf32 += *aPos++; myCharUtf32 <<= 6;
        case 0: myCharUtf32 += *aPos++;
    }
    myCharUtf32 -= offsetsFromUTF8[aBytesToRead];
    myPosNext = (Type* )aPos;
}

// magic numbers
template<typename Type> const unsigned long StUtfIterator<Type>::UTF8_BYTE_MASK = 0xBF;
template<typename Type> const unsigned long StUtfIterator<Type>::UTF8_BYTE_MARK = 0x80;
template<typename Type> const unsigned long StUtfIterator<Type>::UTF16_SURROGATE_HIGH_START = 0xD800;
template<typename Type> const unsigned long StUtfIterator<Type>::UTF16_SURROGATE_HIGH_END   = 0xDBFF;
template<typename Type> const unsigned long StUtfIterator<Type>::UTF16_SURROGATE_LOW_START  = 0xDC00;
template<typename Type> const unsigned long StUtfIterator<Type>::UTF16_SURROGATE_LOW_END    = 0xDFFF;
template<typename Type> const unsigned long StUtfIterator<Type>::UTF16_SURROGATE_HIGH_SHIFT = 10;
template<typename Type> const unsigned long StUtfIterator<Type>::UTF16_SURROGATE_LOW_BASE   = 0x0010000UL;
template<typename Type> const unsigned long StUtfIterator<Type>::UTF16_SURROGATE_LOW_MASK   = 0x3FFUL;
template<typename Type> const unsigned long StUtfIterator<Type>::UTF32_MAX_BMP   = 0x0000FFFFUL;
template<typename Type> const unsigned long StUtfIterator<Type>::UTF32_MAX_LEGAL = 0x0010FFFFUL;

template<typename Type> inline
void StUtfIterator<Type>::readUTF16() {
    stUtf32_t aChar = *myPosNext++;
    // if we have the first half of the surrogate pair
    if(aChar >= UTF16_SURROGATE_HIGH_START
    && aChar <= UTF16_SURROGATE_HIGH_END) {
        const stUtf32_t aChar2 = *myPosNext;
        // complete the surrogate pair
        if(aChar2 >= UTF16_SURROGATE_LOW_START
        && aChar2 <= UTF16_SURROGATE_LOW_END) {
            aChar = ((aChar - UTF16_SURROGATE_HIGH_START) << UTF16_SURROGATE_HIGH_SHIFT)
                  + (aChar2 - UTF16_SURROGATE_LOW_START)   + UTF16_SURROGATE_LOW_BASE;
            ++myPosNext;
        }
    }
    myCharUtf32 = aChar;
}

template<typename Type> inline
size_t StUtfIterator<Type>::getAdvanceBytesUtf8() const {
    if(myCharUtf32 >= UTF16_SURROGATE_HIGH_START
    && myCharUtf32 <= UTF16_SURROGATE_LOW_END) {
        // UTF-16 surrogate values are illegal in UTF-32
        return 0;
    } else if(myCharUtf32 < stUtf32_t(0x80)) {
        return 1;
    } else if(myCharUtf32 < stUtf32_t(0x800)) {
        return 2;
    } else if(myCharUtf32 < stUtf32_t(0x10000)) {
        return 3;
    } else if(myCharUtf32 <= UTF32_MAX_LEGAL) {
        return 4;
    } else {
        // illegal
        return 0;
    }
}

template<typename Type> inline
stUtf8_t* StUtfIterator<Type>::getUtf8(stUtf8_t* theBuffer) const {
    // unsigned arithmetic used
    return (stUtf8_t* )getUtf8((stUtf8u_t* )theBuffer);
}

template<typename Type> inline
stUtf8u_t* StUtfIterator<Type>::getUtf8(stUtf8u_t* theBuffer) const {
    stUtf32_t aChar = myCharUtf32;
    if(myCharUtf32 >= UTF16_SURROGATE_HIGH_START
    && myCharUtf32 <= UTF16_SURROGATE_LOW_END) {
        // UTF-16 surrogate values are illegal in UTF-32
        return theBuffer;
    } else if(myCharUtf32 < stUtf32_t(0x80)) {
        *theBuffer++ = stUtf8u_t (aChar | UTF8_FIRST_BYTE_MARK[1]);
        return theBuffer;
    } else if(myCharUtf32 < stUtf32_t(0x800)) {
        *++theBuffer = stUtf8u_t((aChar | UTF8_BYTE_MARK) & UTF8_BYTE_MASK); aChar >>= 6;
        *--theBuffer = stUtf8u_t (aChar | UTF8_FIRST_BYTE_MARK[2]);
        return theBuffer + 2;
    } else if(myCharUtf32 < stUtf32_t(0x10000)) {
        theBuffer += 3;
        *--theBuffer = stUtf8u_t((aChar | UTF8_BYTE_MARK) & UTF8_BYTE_MASK); aChar >>= 6;
        *--theBuffer = stUtf8u_t((aChar | UTF8_BYTE_MARK) & UTF8_BYTE_MASK); aChar >>= 6;
        *--theBuffer = stUtf8u_t (aChar | UTF8_FIRST_BYTE_MARK[3]);
        return theBuffer + 3;
    } else if(myCharUtf32 <= UTF32_MAX_LEGAL) {
        theBuffer += 4;
        *--theBuffer = stUtf8u_t((aChar | UTF8_BYTE_MARK) & UTF8_BYTE_MASK); aChar >>= 6;
        *--theBuffer = stUtf8u_t((aChar | UTF8_BYTE_MARK) & UTF8_BYTE_MASK); aChar >>= 6;
        *--theBuffer = stUtf8u_t((aChar | UTF8_BYTE_MARK) & UTF8_BYTE_MASK); aChar >>= 6;
        *--theBuffer = stUtf8u_t (aChar | UTF8_FIRST_BYTE_MARK[4]);
        return theBuffer + 4;
    } else {
        // illegal
        return theBuffer;
    }
}

template<typename Type> inline
size_t StUtfIterator<Type>::getAdvanceBytesUtf16() const {
    if(myCharUtf32 <= UTF32_MAX_BMP) { // target is a character <= 0xFFFF
        // UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values
        if(myCharUtf32 >= UTF16_SURROGATE_HIGH_START
        && myCharUtf32 <= UTF16_SURROGATE_LOW_END) {
            return 0;
        } else {
            return sizeof(stUtf16_t);
        }
    } else if(myCharUtf32 > UTF32_MAX_LEGAL) {
        // illegal
        return 0;
    } else {
        // target is a character in range 0xFFFF - 0x10FFFF
        // surrogate pair
        return sizeof(stUtf16_t) * 2;
    }
}

template<typename Type> inline
stUtf16_t* StUtfIterator<Type>::getUtf16(stUtf16_t* theBuffer) const {
    if(myCharUtf32 <= UTF32_MAX_BMP) { // target is a character <= 0xFFFF
        // UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values
        if(myCharUtf32 >= UTF16_SURROGATE_HIGH_START
        && myCharUtf32 <= UTF16_SURROGATE_LOW_END) {
            return theBuffer;
        } else {
            *theBuffer++ = stUtf16_t(myCharUtf32);
            return theBuffer;
        }
    } else if(myCharUtf32 > UTF32_MAX_LEGAL) {
        // illegal
        return theBuffer;
    } else {
        // surrogate pair
        stUtf32_t aChar = myCharUtf32 - UTF16_SURROGATE_LOW_BASE;
        *theBuffer++ = stUtf16_t((aChar >> UTF16_SURROGATE_HIGH_SHIFT) + UTF16_SURROGATE_HIGH_START);
        *theBuffer++ = stUtf16_t((aChar &  UTF16_SURROGATE_LOW_MASK)   + UTF16_SURROGATE_LOW_START);
        return theBuffer;
    }
}

template<typename Type> inline
stUtf32_t* StUtfIterator<Type>::getUtf32(stUtf32_t* theBuffer) const {
    *theBuffer++ = myCharUtf32;
    return theBuffer;
}

template<typename Type> template<typename TypeWrite> inline
size_t StUtfIterator<Type>::getAdvanceBytesUtf() const {
    switch(sizeof(TypeWrite)) {
        case sizeof(stUtf8_t):  return getAdvanceBytesUtf8();
        case sizeof(stUtf16_t): return getAdvanceBytesUtf16();
        case sizeof(stUtf32_t): return getAdvanceBytesUtf32();
        default:                return 0; // invalid case
    }
}

template<typename Type> template<typename TypeWrite> inline
TypeWrite* StUtfIterator<Type>::getUtf(TypeWrite* theBuffer) const {
    switch(sizeof(TypeWrite)) {
        case sizeof(stUtf8_t):  return (TypeWrite* )getUtf8 ((stUtf8u_t* )theBuffer);
        case sizeof(stUtf16_t): return (TypeWrite* )getUtf16((stUtf16_t* )theBuffer);
        case sizeof(stUtf32_t): return (TypeWrite* )getUtf32((stUtf32_t* )theBuffer);
        default:                return NULL; // invalid case
    }
}

#endif //__StUtfIterator_inl_
