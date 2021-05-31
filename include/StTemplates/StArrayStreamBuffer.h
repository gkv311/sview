/**
 * Copyright © 2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StArrayStreamBuffer_h__
#define __StArrayStreamBuffer_h__

#include <stTypes.h>

#include <fstream>

/**
 * Custom buffer for streaming from allocated memory.
 */
class StArrayStreamBuffer : public std::streambuf {

        public:

    //! Main constructor.
    StArrayStreamBuffer(const char*  theBegin,
                        const size_t theSize)
    : myBegin  (theBegin),
      myEnd    (theBegin + theSize),
      myCurrent(theBegin) {}

        private:

    /**
     * Get character on underflow.
     * Virtual function called by other member functions to get the current character
     * in the controlled input sequence without changing the current position.
     */
    virtual int_type underflow() ST_ATTR_OVERRIDE {
        if(myCurrent == myEnd) {
            return traits_type::eof();
        }

        return traits_type::to_int_type(*myCurrent);
    }

    /**
     * Get character on underflow and advance position.
     * Virtual function called by other member functions to get the current character
     * in the controlled input sequence and then advance the position indicator to the next character.
     */
    virtual int_type uflow() ST_ATTR_OVERRIDE {
        if(myCurrent == myEnd) {
            return traits_type::eof();
        }

        return traits_type::to_int_type(*myCurrent++);
    }

    /**
     * Put character back in the case of backup underflow.
     * Virtual function called by other member functions to put a character back
     * into the controlled input sequence and decrease the position indicator.
     */
    virtual int_type pbackfail(int_type ch) ST_ATTR_OVERRIDE {
        if(myCurrent == myBegin
        || (ch != traits_type::eof()
         && ch != myCurrent[-1])) {
            return traits_type::eof();
        }

        return traits_type::to_int_type(*--myCurrent);
    }

    /**
     * Get number of characters available.
     * Virtual function (to be read s-how-many-c) called by other member functions
     * to get an estimate on the number of characters available in the associated input sequence.
     */
    virtual std::streamsize showmanyc() ST_ATTR_OVERRIDE {
        if(myCurrent > myEnd) {
            // assert
        }
        return myEnd - myCurrent;
    }

    /**
     * Seek to specified position.
     */
    virtual std::streampos seekoff(std::streamoff theOff,
                                   std::ios_base::seekdir theWay,
                                   std::ios_base::openmode theWhich) ST_ATTR_OVERRIDE {
        switch(theWay) {
            case std::ios_base::beg: {
                myCurrent = myBegin + theOff;
                if(myCurrent >= myEnd) {
                    myCurrent = myEnd;
                }
                break;
            }
            case std::ios_base::cur: {
                myCurrent += theOff;
                if(myCurrent >= myEnd) {
                    myCurrent = myEnd;
                }
                break;
            }
            case std::ios_base::end: {
                myCurrent = myEnd - theOff;
                if(myCurrent < myBegin) {
                    myCurrent = myBegin;
                }
                break;
            }
            default: {
                break;
            }
        }
        (void )theWhich;
        return myCurrent - myBegin;
    }

        public:

    /**
     * Read a bunch of bytes at once.
     */
    virtual std::streamsize xsgetn(char* thePtr,
                                   std::streamsize theCount) ST_ATTR_OVERRIDE {
        const char* aCurrent = myCurrent + theCount;
        if(aCurrent >= myEnd) {
            aCurrent = myEnd;
        }
        size_t aCopied = aCurrent - myCurrent;
        if(aCopied == 0) {
            return 0;
        }
        memcpy(thePtr, myCurrent, aCopied);
        myCurrent = aCurrent;
        return aCopied;
    }

        private:

    // copying is not allowed
    StArrayStreamBuffer           (const StArrayStreamBuffer& );
    StArrayStreamBuffer& operator=(const StArrayStreamBuffer& );

        private:

    const char* const myBegin;
    const char* const myEnd;
    const char* myCurrent;

};


#endif // __StArrayStreamBuffer_h__
