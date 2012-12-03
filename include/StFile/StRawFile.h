/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
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
class ST_LOCAL StRawFile : public StFileNode {

        public:

    typedef enum tagReadWrite {
        READ,
        WRITE,
    } ReadWrite;

        private:

    FILE*  myFileHandle;
    stUByte_t* myBuffer;
    size_t   myBuffSize;

        public:

    StRawFile(const StString& theFilePath = StString(),
              StNode* theParentNode = NULL);

    virtual ~StRawFile();

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
    void initBuffer(size_t theDataSize);

    /**
     * Free current buffer.
     */
    void freeBuffer();

    /**
     * Returns true if file is opened.
     */
    bool isOpen() const {
        return myFileHandle != NULL;
    }

    /**
     * Open the file handle for read or write operation.
     */
    bool openFile(StRawFile::ReadWrite theFlags,
                  const StString& theFilePath = StString());

    /**
     * Close file handle
     */
    void closeFile();

    /**
     * Write current buffer content to the file.
     * If size not set - whole buffer will be saved.
     */
    size_t writeFile(size_t theBytes = 0);

    /**
     * Fill the buffer with file content.
     * @param theFilePath (const StString& ) - the file path;
     * @return true if file was read.
     */
    bool readFile(const StString& theFilePath = StString());

    /**
     * Write the buffer into the file.
     * @param theFilePath (const StString& ) - the file path;
     * @return true if file was stored.
     */
    bool saveFile(const StString& theFilePath = StString());

    /**
     * Read the text file and return it as a string.
     */
    static StString readTextFile(const StString& theFilePath);

};

#endif //__StRawFile_h__
