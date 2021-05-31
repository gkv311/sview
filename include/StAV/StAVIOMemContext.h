/**
 * Copyright © 2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StAVIOMemContext_h_
#define __StAVIOMemContext_h_

#include <StAV/StAVIOContext.h>

/**
 * Custom AVIO context for the file.
 */
class StAVIOMemContext : public StAVIOContext {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StAVIOMemContext();

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StAVIOMemContext();

    /**
     * Close the context.
     */
    ST_CPPEXPORT void close();

    /**
     * Wrap existing buffer.
     * Buffer should not be released by caller until closing the context.
     */
    ST_CPPEXPORT void wrapBuffer(uint8_t* theSrcBuffer,
                                 size_t   theSrcSize);

    /**
     * Read from the file.
     */
    ST_CPPEXPORT virtual int read(uint8_t* theBuf,
                                  int      theBufSize) ST_ATTR_OVERRIDE;

    /**
     * Write into the file.
     */
    ST_CPPEXPORT virtual int write(uint8_t* theBuf,
                                   int      theBufSize) ST_ATTR_OVERRIDE;

    /**
     * Seek within the file.
     */
    ST_CPPEXPORT virtual int64_t seek(int64_t theOffset,
                                      int     theWhence) ST_ATTR_OVERRIDE;

        protected:

    uint8_t* mySrcBuffer; //!< memory buffer
    int64_t  mySrcSize;   //!< buffer size
    int64_t  myPosition;  //!< current position within the buffer

};

ST_DEFINE_HANDLE(StAVIOMemContext, StAVIOContext);

#endif // __StAVIOMemContext_h_
