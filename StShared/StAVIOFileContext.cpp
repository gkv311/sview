/**
 * Copyright © 2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StAV/StAVIOFileContext.h>

StAVIOFileContext::StAVIOFileContext()
: myFile(NULL) {
    //
}

StAVIOFileContext::~StAVIOFileContext() {
    close();
}

void StAVIOFileContext::close() {
    if(myFile != NULL) {
        fclose(myFile);
        myFile = NULL;
    }
}

bool StAVIOFileContext::openFromDescriptor(int theFD, const char* theMode) {
    close();
#ifdef _WIN32
    myFile = ::_fdopen(theFD, theMode);
#else
    myFile =  ::fdopen(theFD, theMode);
#endif
    return myFile != NULL;
}

int StAVIOFileContext::read(uint8_t* theBuf,
                            int      theBufSize) {

    int aNbRead = myFile != NULL
                ? (int )::fread(theBuf, 1, theBufSize, myFile)
                : 0;
    return aNbRead;
}

int StAVIOFileContext::write(uint8_t* theBuf,
                             int      theBufSize) {
    int aNbWritten = myFile != NULL
                   ? (int )::fwrite(theBuf, 1, theBufSize, myFile)
                   : 0;
    return aNbWritten;
}

int64_t StAVIOFileContext::seek(int64_t theOffset,
                                int     theWhence) {
    if(theWhence == AVSEEK_SIZE
    || myFile == NULL) {
        return -1;
    }

#ifdef _WIN32
    bool isOk = ::_fseeki64(myFile, theOffset, theWhence) == 0;
#else
    bool isOk =    ::fseeko(myFile, theOffset, theWhence) == 0;
#endif
    if(!isOk) {
        return -1;
    }

#ifdef _WIN32
    return ::_ftelli64(myFile);
#else
    return ::ftello(myFile);
#endif
}
