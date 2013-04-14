/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StRawFile_h__
#define __StRawFile_h__

#include <StFile/StFileNode.h>

/**
 * @class class to access to the small binary and text files.
 */
class StRawFile : public StFileNode {

        public:

    typedef enum tagReadWrite {
        READ,
        WRITE,
    } ReadWrite;

        public:

    ST_CPPEXPORT StRawFile(const StString& theFilePath = StString(),
                           StNode* theParentNode = NULL);

    ST_CPPEXPORT virtual ~StRawFile();

    /**
     * Access to the raw buffer.
     */
    const stUByte_t* getBuffer() const {
        return myBuffer;
    }

    /**
     * Access to the raw buffer.
     */
    stUByte_t* changeBuffer() {
        return myBuffer;
    }

    /**
     * @return the buffer size in bytes.
     */
    size_t getSize() const {
        return myBuffSize;
    }

    /**
     * Casts the raw buffer as string.
     */
    StString getAsANSIText() const {
        return StString((const char* )myBuffer);
    }

    /**
     * (Re)initialize the buffer.
     * @param theDataSize (size_t ) - new buffer size in bytes.
     */
    ST_CPPEXPORT void initBuffer(size_t theDataSize);

    /**
     * Free current buffer.
     */
    ST_CPPEXPORT void freeBuffer();

    /**
     * Returns true if file is opened.
     */
    bool isOpen() const {
        return myFileHandle != NULL;
    }

    /**
     * Open the file handle for read or write operation.
     */
    ST_CPPEXPORT bool openFile(StRawFile::ReadWrite theFlags,
                               const StString& theFilePath = StString());

    /**
     * Close file handle
     */
    ST_CPPEXPORT void closeFile();

    /**
     * Write current buffer content to the file.
     * If size not set - whole buffer will be saved.
     */
    ST_CPPEXPORT size_t writeFile(size_t theBytes = 0);

    /**
     * Fill the buffer with file content.
     * @param theFilePath (const StString& ) - the file path;
     * @return true if file was read.
     */
    ST_CPPEXPORT bool readFile(const StString& theFilePath = StString());

    /**
     * Write the buffer into the file.
     * @param theFilePath (const StString& ) - the file path;
     * @return true if file was stored.
     */
    ST_CPPEXPORT bool saveFile(const StString& theFilePath = StString());

    /**
     * Read the text file and return it as a string.
     */
    ST_CPPEXPORT static StString readTextFile(const StString& theFilePath);

        private:

    FILE*      myFileHandle;
    stUByte_t* myBuffer;
    size_t     myBuffSize;

};

#endif //__StRawFile_h__
