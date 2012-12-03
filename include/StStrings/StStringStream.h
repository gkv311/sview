/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StStringStream_h__
#define __StStringStream_h__

#include <sstream>
#include <locale>

/**
 * This class defines own std::stringstream template.
 * The main reason for replacement is that original template can not be easily
 * used over alien string buffer.
 *
 * This is the class draft which will be improved when the usage scenarios
 * will be defined. Currently it defined just to read numbers from strings
 * using C locale.
 */
class ST_LOCAL StStringStream : public std::basic_iostream< char, std::char_traits<char> > {

        private:

    /**
     * Defines simple stream buffer which should be initialized over alien buffer.
     */
    typedef std::basic_iostream< char, std::char_traits<char> > base;
    class IStreamBuf : public std::basic_streambuf< char, std::char_traits<char> > {

            public:

        IStreamBuf(char* theBuffer, std::streamsize theBufferLength) {
            setupInBuffer(theBuffer, theBufferLength);
        }

        void setupInBuffer(char* theBuffer, std::streamsize theBufferLength) {
            setg(theBuffer, theBuffer, theBuffer + theBufferLength);
        }

    } myStringBuffer;

        public:

    /**
     * Initialize the empty stream.
     */
    StStringStream()
    : base(&myStringBuffer),
      myStringBuffer(NULL, 0) {
        //
    }

    /**
     * Destructor.
     */
    virtual ~StStringStream() {}

    /**
     * Use C locale for numbers interpretations instead of global locale.
     * This is preferred for system-independent format.
     */
    void setCLocale() {
        imbue(std::locale("C"));
    }

    /**
     * Setup alien buffer for reading.
     */
    void setInputBuffer(char* theBuffer, std::streamsize theBufferLength) {
        myStringBuffer.setupInBuffer(theBuffer, theBufferLength);
        clear(); // reset flags
    }

};

#endif //__StStringStream_h__
