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
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StUtfIterator_h_
#define __StUtfIterator_h_

#include <stTypes.h>

/**
 * This is a major template class for Unicode strings support.
 * It defines an iterator and provide correct way to read multi-byte text (UTF-8 and UTF-16)
 * and convert it from one to another.
 * The current value of iterator returned as UTF-32 Unicode code.
 *
 * Example of the string which requires surrogate pairs in UTF-16:
 * "𐐐𐐄𐐢𐐆𐐤𐐝 𐐓𐐅 𐐜 𐐢𐐃𐐡𐐔" ("Holiness to the Lord") in the Deseret alphabet.
 */
template<typename Type>
class StUtfIterator {

        private:

    const Type* myPosition;  //!< buffer position of the first element in the current character
    const Type* myPosNext;   //!< buffer position of the first element in the next character
    size_t      myCharIndex; //!< index displacement from iterator intialization
    stUtf32_t   myCharUtf32; //!< character stored at the current buffer position

        public:

    /**
     * Constructor.
     * @param theString (const Type* ) - buffer to iterate.
     */
    inline StUtfIterator(const Type* theString)
    : myPosition(theString),
      myPosNext(theString),
      myCharIndex(0),
      myCharUtf32(0) {
        if(theString != NULL) {
            ++(*this);
            myCharIndex = 0;
        }
    };

    /**
     * Initialize iterator within specified string.
     */
    inline void init(const Type* theString) {
        myPosition  = theString;
        myPosNext   = theString;
        myCharUtf32 = 0;
        if(theString != NULL) {
            ++(*this);
        }
        myCharIndex = 0;
    }

    /**
     * Pre-increment operator. Reads the next unicode character.
     * Note - not protected against overruns.
     */
    inline StUtfIterator& operator++() {
        myPosition = myPosNext;
        ++myCharIndex;
        switch(sizeof(Type)) {
            case 1: readUTF8();  break;
            case 2: readUTF16(); break;
            case 4: // UTF-32
            default:
                myCharUtf32 = *myPosNext++;
        }
        return *this;
    }

    /**
     * Post-increment operator.
     * Note - not protected against overruns.
     */
    inline StUtfIterator operator++(int ) {
        StUtfIterator aCopy = *this;
        ++*this;
        return aCopy;
    }

    /**
     * Several characters forward.
     */
    inline StUtfIterator& operator+=(int theIncrement) {
        for(; theIncrement > 0; --theIncrement) {
            ++*this;
        }
        return *this;
    }

    /**
     * Equality operator.
     */
    inline bool operator==(const StUtfIterator& theRight) const {
        return myPosition == theRight.myPosition;
    }

    /**
     * Dereference operator.
     * @return the UTF-32 codepoint of the character currently pointed by iterator.
     */
    inline stUtf32_t operator*() const {
        return myCharUtf32;
    }

    /**
     * Buffer-fetching getter.
     */
    inline const Type* getBufferHere() const { return myPosition; }

    /**
     * Buffer-fetching getter. Dangerous! Iterator should be reinitialized on buffer change.
     */
    inline Type* changeBufferHere() { return (Type* )myPosition; }

    /**
     * Buffer-fetching getter.
     */
    inline const Type* getBufferNext() const { return myPosNext; }

    /**
     * Return the index displacement from iterator initialization.
     */
    inline size_t getIndex() const {
        return myCharIndex;
    }

    /**
     * @return the advance in bytes to store current symbol in UTF-8.
     *  0 means an invalid symbol;
     *  1-4 bytes are valid range.
     */
    size_t getAdvanceBytesUtf8() const;

    /**
     * @return the advance in bytes to store current symbol in UTF-16.
     *  0 means an invalid symbol;
     *  2 bytes is a general case;
     *  4 bytes for surrogate pair.
     */
    size_t getAdvanceBytesUtf16() const;

    /**
     * @return the advance in bytes to store current symbol in UTF-32.
     * Always 4 bytes (method for consistency).
     */
    inline size_t getAdvanceBytesUtf32() const {
        return sizeof(stUtf32_t);
    }

    /**
     * Fill the UTF-8 buffer within current Unicode symbol.
     * Use method getAdvanceUtf8() to allocate buffer with enough size.
     * @param theBuffer (stUtf8_t* ) - buffer to fill;
     * @return new buffer position (for next char).
     */
    stUtf8_t*  getUtf8(stUtf8_t*  theBuffer) const;
    stUtf8u_t* getUtf8(stUtf8u_t* theBuffer) const;

    /**
     * Fill the UTF-16 buffer within current Unicode symbol.
     * Use method getAdvanceUtf16() to allocate buffer with enough size.
     * @param theBuffer (stUtf16_t* ) - buffer to fill;
     * @return new buffer position (for next char).
     */
    stUtf16_t* getUtf16(stUtf16_t* theBuffer) const;

    /**
     * Fill the UTF-32 buffer within current Unicode symbol.
     * Use method getAdvanceUtf32() to allocate buffer with enough size.
     * @param theBuffer (stUtf32_t* ) - buffer to fill;
     * @return new buffer position (for next char).
     */
    stUtf32_t* getUtf32(stUtf32_t* theBuffer) const;

    /**
     * @return the advance in TypeWrite chars needed to store current symbol.
     */
    template<typename TypeWrite>
    size_t getAdvanceBytesUtf() const;

    /**
     * Fill the UTF-** buffer within current Unicode symbol.
     * Use method getAdvanceUtf() to allocate buffer with enough size.
     * @param theBuffer (TypeWrite* ) - buffer to fill;
     * @return new buffer position (for next char).
     */
    template<typename TypeWrite>
    TypeWrite* getUtf(TypeWrite* theBuffer) const;

        private:

    /**
     * Helper function for reading a single UTF8 character from the string.
     * Updates internal state appropriately.
     */
    void readUTF8();

    /**
     * Helper function for reading a single UTF16 character from the string.
     * Updates internal state appropriately.
     */
    void readUTF16();

        private: //!< unicode magic numbers

    static const unsigned char UTF8_BYTES_MINUS_ONE[256];
    static const unsigned long offsetsFromUTF8[6];
    static const unsigned char UTF8_FIRST_BYTE_MARK[7];
    static const unsigned long UTF8_BYTE_MASK;
    static const unsigned long UTF8_BYTE_MARK;
    static const unsigned long UTF16_SURROGATE_HIGH_START;
    static const unsigned long UTF16_SURROGATE_HIGH_END;
    static const unsigned long UTF16_SURROGATE_LOW_START;
    static const unsigned long UTF16_SURROGATE_LOW_END;
    static const unsigned long UTF16_SURROGATE_HIGH_SHIFT;
    static const unsigned long UTF16_SURROGATE_LOW_BASE;
    static const unsigned long UTF16_SURROGATE_LOW_MASK;
    static const unsigned long UTF32_MAX_BMP;
    static const unsigned long UTF32_MAX_LEGAL;

};

typedef StUtfIterator<stUtf8_t>    StUtf8Iter;
typedef StUtfIterator<stUtf16_t>   StUtf16Iter;
typedef StUtfIterator<stUtf32_t>   StUtf32Iter;
typedef StUtfIterator<stUtfWide_t> StUtfWideIter;

// template implementation
#include "StUtfIterator.inl"

#endif //__StUtfIterator_h_
