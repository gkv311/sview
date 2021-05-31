/**
 * Copyright © 2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StAVIOFileContext_h_
#define __StAVIOFileContext_h_

#include <StAV/StAVIOContext.h>

/**
 * Custom AVIO context for the file.
 */
class StAVIOFileContext : public StAVIOContext {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StAVIOFileContext();

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StAVIOFileContext();

    /**
     * Close the file.
     */
    ST_CPPEXPORT void close();

    /**
     * Associate a stream with a file that was previously opened for low-level I/O.
     * The associated file will be automatically closed on destruction.
     */
    ST_CPPEXPORT bool openFromDescriptor(int theFD, const char* theMode);

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

    FILE* myFile;

};

ST_DEFINE_HANDLE(StAVIOFileContext, StAVIOContext);

#endif // __StAVIOFileContext_h_
