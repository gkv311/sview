/**
 * Copyright © 2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StAV/StAVIOContext.h>

namespace {

    /**
     * Callback for reading the data.
     */
    static int readCallback(void*    theOpaque,
                            uint8_t* theBuf,
                            int      theBufSize) {
        return theOpaque != NULL
             ? ((StAVIOContext* )theOpaque)->read(theBuf, theBufSize)
             : 0;
    }

    /**
     * Callback for writing the data.
     */
    static int writeCallback(void*    theOpaque,
                             uint8_t* theBuf,
                             int      theBufSize) {
        return theOpaque != NULL
             ? ((StAVIOContext* )theOpaque)->write(theBuf, theBufSize)
             : 0;
    }

    /**
     * Callback for seeking to new position.
     */
    static int64_t seekCallback(void*   theOpaque,
                                int64_t theOffset,
                                int     theWhence) {
       return theOpaque != NULL
             ? ((StAVIOContext* )theOpaque)->seek(theOffset, theWhence)
             : -1;
    }

}

StAVIOContext::StAVIOContext()
: myAvioCtx(NULL) {
    const int aBufferSize = 32768;
    unsigned char* aBufferIO = (unsigned char* )av_malloc(aBufferSize + AV_INPUT_BUFFER_PADDING_SIZE);
    myAvioCtx = avio_alloc_context(aBufferIO, aBufferSize, 0, this, readCallback, writeCallback, seekCallback);
}

StAVIOContext::~StAVIOContext() {
    if(myAvioCtx != NULL) {
        av_free(myAvioCtx);
    }
}
