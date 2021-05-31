/**
 * Copyright © 2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StAV/StAVIOFileContext.h>

extern "C" {
    #include <libavutil/error.h>
};

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

    if(myFile == NULL) {
        return -1;
    }

    int aNbRead = (int )::fread(theBuf, 1, theBufSize, myFile);
    if(aNbRead == 0
    && feof(myFile) != 0) {
        return AVERROR_EOF;
    }

    return aNbRead;
}

int StAVIOFileContext::write(uint8_t* theBuf,
                             int      theBufSize) {
    if(myFile == NULL) {
        return -1;
    }

    return (int )::fwrite(theBuf, 1, theBufSize, myFile);
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
