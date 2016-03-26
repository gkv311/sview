/**
 * Copyright © 2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StAV/StAVIOMemContext.h>

StAVIOMemContext::StAVIOMemContext()
: mySrcBuffer(NULL),
  mySrcSize(0),
  myPosition(0) {
    //
}

StAVIOMemContext::~StAVIOMemContext() {
    close();
}

void StAVIOMemContext::close() {
    mySrcBuffer = NULL;
    mySrcSize   = 0;
    myPosition  = 0;
}

void StAVIOMemContext::wrapBuffer(uint8_t* theSrcBuffer,
                                  size_t   theSrcSize) {
    close();
    mySrcBuffer = theSrcBuffer;
    mySrcSize   = theSrcSize;
}

int StAVIOMemContext::read(uint8_t* theBuf,
                           int      theBufSize) {
    if(theBuf == NULL
    || theBufSize <= 0
    || mySrcBuffer == NULL
    || mySrcSize == 0) {
        return 0;
    }

    int aNbRead = theBufSize;
    if(myPosition + theBufSize > mySrcSize) {
        aNbRead = mySrcSize - myPosition;
    }
    stMemCpy(theBuf, mySrcBuffer + myPosition, aNbRead);
    myPosition += aNbRead;
    return aNbRead;
}

int StAVIOMemContext::write(uint8_t* theBuf,
                            int      theBufSize) {
    if(theBuf == NULL
    || theBufSize <= 0
    || mySrcBuffer == NULL
    || mySrcSize == 0) {
        return 0;
    }

    int aNbWritten = theBufSize;
    if(myPosition + theBufSize > mySrcSize) {
        aNbWritten = mySrcSize - myPosition;
    }
    stMemCpy(mySrcBuffer + myPosition, theBuf, aNbWritten);
    myPosition += aNbWritten;
    return aNbWritten;
}

int64_t StAVIOMemContext::seek(int64_t theOffset,
                               int     theWhence) {
    if(theWhence == AVSEEK_SIZE) {
        return mySrcSize;
    }
    if(mySrcBuffer == NULL
    || mySrcSize == 0) {
        return -1;
    }

    if(theWhence == SEEK_SET) {
        myPosition = theOffset;
    } else if(theWhence == SEEK_CUR) {
        myPosition += theOffset;
    } else if(theWhence == SEEK_END) {
        myPosition = mySrcSize + theOffset;
    }

    if(myPosition < 0) {
        myPosition = 0;
    } else if(myPosition > mySrcSize) {
        myPosition = mySrcSize;
    }
    return myPosition;
}
