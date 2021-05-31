/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <cstdio> // forward-declared to work-around broken MinGW headers

#include <StFile/StRawFile.h>
#include <StAV/stAV.h>
#include <StStrings/StLogger.h>

#include <iostream>
#include <fstream>
#include <limits>

#if defined(_WIN32)
    #define ftell64(a)     _ftelli64(a)
    #define fseek64(a,b,c) _fseeki64(a,b,c)
#else
    #define ftell64(a)     ftello(a)
    #define fseek64(a,b,c) fseeko(a,b,c)
#endif

#ifdef max
    #undef max
#endif

int StRawFile::avInterruptCallback(void* thePtr) {
    StRawFile* aRawFile = reinterpret_cast<StRawFile*>(thePtr);
    return aRawFile != NULL
         ? aRawFile->onInterrupted()
         : 0;
}

StRawFile::StRawFile(const StCString& theFilePath,
                     StNode*          theParent)
: StFileNode(theFilePath, theParent),
  myContextIO(NULL),
  myFileHandle(NULL),
  myBuffer(NULL),
  myBuffSize(0),
  myLength(0),
  myIsOwnData(false) {
    //
}

StRawFile::~StRawFile() {
    closeFile();
    freeBuffer();
}

bool StRawFile::openFile(StRawFile::ReadWrite theFlags,
                         const StCString&     theFilePath,
                         const int            theOpenedFd) {
    // close previously opened file handle if any
    closeFile();

    if(!theFilePath.isEmpty()) {
        setSubPath(theFilePath);
    }

    if(theOpenedFd != -1) {
    #ifdef _WIN32
        myFileHandle = ::_fdopen(theOpenedFd, (theFlags == StRawFile::WRITE) ?  "wb" :  "rb");
    #else
        myFileHandle =  ::fdopen(theOpenedFd, (theFlags == StRawFile::WRITE) ?  "wb" :  "rb");
    #endif
        return myFileHandle != NULL;
    }

    StString aFilePath = getPath();
#if(LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 21, 0))
    if(StFileNode::isRemoteProtocolPath(aFilePath)
    && stAV::init()) {
        AVIOInterruptCB anInterruptCB;
        stMemZero(&anInterruptCB, sizeof(anInterruptCB));
        anInterruptCB.callback = &StRawFile::avInterruptCallback;
        anInterruptCB.opaque   = this;
        const int aResult = avio_open2(&myContextIO,
                                       aFilePath.toCString(),
                                       (theFlags == StRawFile::WRITE) ? AVIO_FLAG_WRITE : AVIO_FLAG_READ,
                                       &anInterruptCB,
                                       NULL);
        if(aResult < 0) {
            ST_ERROR_LOG("StRawFile, avio_open2(" + aFilePath + ") failed - " + stAV::getAVErrorDescription(aResult));
            return false;
        }
        return true;
    }
#endif

#ifdef _WIN32
    StStringUtfWide aPathWide;
    aPathWide.fromUnicode(aFilePath);
    myFileHandle = _wfopen(aPathWide.toCString(), (theFlags == StRawFile::WRITE) ? L"wb" : L"rb");
#else
    myFileHandle =   fopen(aFilePath.toCString(), (theFlags == StRawFile::WRITE) ?  "wb" :  "rb");
#endif

    return myFileHandle != NULL;
}

void StRawFile::closeFile() {
    if(myContextIO != NULL) {
        avio_close(myContextIO);
        myContextIO = NULL;
    }
    if(myFileHandle != NULL) {
        fclose(myFileHandle);
        myFileHandle = NULL;
    }
}

void StRawFile::initBuffer(size_t theDataSize) {
    if(myBuffSize >= theDataSize) {
        myBuffSize = theDataSize;
        return;
    }
    freeBuffer();
    myIsOwnData = true;
    myBuffSize = theDataSize;
    myBuffer = stMemAllocAligned<stUByte_t*>(myBuffSize + 1);
    myBuffer[myBuffSize] = '\0'; // just for safe
}

void StRawFile::wrapBuffer(stUByte_t* theBuffer,
                           size_t theDataSize) {
    freeBuffer();
    myIsOwnData = false;
    myBuffSize = theDataSize;
    myBuffer = theBuffer;
}

void StRawFile::freeBuffer() {
    if(myIsOwnData) {
        stMemFreeAligned(myBuffer);
        myIsOwnData = false;
    }
    myBuffer = NULL;
    myBuffSize = 0;
}

bool StRawFile::readFile(const StCString& theFilePath,
                         const int        theOpenedFd,
                         const size_t     theReadMax) {
    freeBuffer();

    if(!openFile(StRawFile::READ, theFilePath, theOpenedFd)) {
        return false;
    }

    if(myContextIO != NULL) {
        int64_t aFileLen = avio_size(myContextIO);
        if(aFileLen > 0) {
            // stream of known size - create a buffer and read the data
            size_t aReadLen = 0;
            if(aFileLen <= int64_t(std::numeric_limits<ptrdiff_t>::max())) {
                aReadLen = theReadMax != 0 ? (size_t )stMin(int64_t(theReadMax), aFileLen) : size_t(aFileLen);
            } else if(theReadMax != 0) {
                aReadLen = theReadMax;
            } else {
                return false;
            }

            initBuffer(aReadLen);
            if(myBuffSize != aReadLen) {
                return false;
            }

            stUByte_t*   aBufferIter = myBuffer;
            size_t       aBytesLeft  = myBuffSize;
            const size_t aChunkLimit = size_t(std::numeric_limits<int>::max());
            for(;;) {
                if(aBytesLeft < aChunkLimit) {
                    const int aResult = avio_read(myContextIO, aBufferIter, int(aBytesLeft));
                    if(aResult < 0) {
                        ST_ERROR_LOG("StRawFile, avio_read() failed - " + stAV::getAVErrorDescription(aResult));
                    }

                    closeFile();
                    return aResult == int(aBytesLeft);
                }

                const int aResult = avio_read(myContextIO, aBufferIter, int(aChunkLimit));
                if(aResult != int(aChunkLimit)) {
                    closeFile();
                    return false;
                }
                aBufferIter += aChunkLimit;
                aBytesLeft  -= aChunkLimit;
            }
        }

        // stream of unknown size - read until first error
        StArrayList<stUByte_t*> aChunks;
        aFileLen = 0;
        const size_t aChunkLimit = 4096;
        bool isOk = true;
        for(;;) {
            stUByte_t* aChunk = stMemAllocAligned<stUByte_t*>(aChunkLimit);
            if(aChunk == NULL) {
                isOk = false;
                break;
            }
            int aReadBytes = avio_read(myContextIO, aChunk, int(aChunkLimit));
            if(aReadBytes <= 0) {
                stMemFreeAligned(aChunk);
                break;
            }

            const uint64_t aReadLen = uint64_t(aFileLen) + uint64_t(aReadBytes);
            if(aReadLen > uint64_t(std::numeric_limits<ptrdiff_t>::max())) {
                isOk = false;
                break;
            }

            aFileLen += aReadBytes;
            aChunks.add(aChunk);
            if(theReadMax != 0
            && aReadLen >= uint64_t(theReadMax)) {
                break;
            }
        }
        closeFile();

        // copy chunks to final destination
        if(isOk) {
            initBuffer(size_t(aFileLen));
            isOk = myBuffSize == size_t(aFileLen);
        }

        stUByte_t* aBufferIter = myBuffer;
        size_t     aBytesLeft  = myBuffSize;
        for(size_t aChunkIter = 0; aChunkIter < aChunks.size(); ++aChunkIter) {
            stUByte_t*   aChunk       = aChunks.changeValue(aChunkIter);
            const size_t aBytesToCopy = stMin(aBytesLeft, aChunkLimit);
            if(aBufferIter != NULL) {
                stMemCpy(aBufferIter, aChunk, aBytesToCopy);
            }
            stMemFreeAligned(aChunk);
            if(aBufferIter != NULL) {
                aBufferIter += aBytesToCopy;
            }
            aBytesLeft -= aBytesToCopy;
        }
        return isOk;
    }

    // determine length of file
    fseek64(myFileHandle, 0, SEEK_END);
    int64_t aFileLen = ftell64(myFileHandle);
    if(aFileLen <= 0L) {
        closeFile();
        return false;
    }

    size_t aReadLen = 0;
    if(aFileLen <= int64_t(std::numeric_limits<ptrdiff_t>::max())) {
        aReadLen = theReadMax != 0 ? (size_t )stMin(int64_t(theReadMax), aFileLen) : size_t(aFileLen);
    } else if(theReadMax != 0) {
        aReadLen = theReadMax;
    } else {
        closeFile();
        return false;
    }

    // create a buffer and read the data
    initBuffer(aReadLen);

    fseek64(myFileHandle, 0, SEEK_SET);
    if(myBuffSize == aReadLen) {
        size_t aCountRead = fread(myBuffer, 1, myBuffSize, myFileHandle);
        (void )aCountRead;
    }
    closeFile();
    return true;
}

bool StRawFile::saveFile(const StCString& theFilePath,
                         const int        theOpenedFd) {
    if(!openFile(StRawFile::WRITE, theFilePath, theOpenedFd)) {
        return false;
    }

    const size_t aSize = (myLength != 0) ? myLength : myBuffSize;
    bool isSuccess = writeFile() == aSize;
    closeFile();
    return isSuccess;
}

size_t StRawFile::write(const char*  theBuffer,
                        const size_t theBytes) {
    if(!isOpen()) {
        return 0;
    }

    if(myContextIO != NULL) {
        const stUByte_t* aBufferIter = (const stUByte_t* )theBuffer;
        size_t           aBytesLeft  = theBytes;
        const size_t     aChunkLimit = size_t(std::numeric_limits<int>::max());
        for(;;) {
            if(aBytesLeft < aChunkLimit) {
                avio_write(myContextIO, aBufferIter, int(aBytesLeft));
                return theBytes;
            }

            avio_write(myContextIO, aBufferIter, int(aChunkLimit));
            aBufferIter += aChunkLimit;
            aBytesLeft  -= aChunkLimit;
        }
    }

    return fwrite(theBuffer, 1, theBytes, myFileHandle);
}

size_t StRawFile::writeFile(size_t theBytes) {
    if(myBuffSize == 0) {
        return 0;
    }

    const size_t aSize = (myLength != 0) ? myLength : myBuffSize;
    return write((const char* )myBuffer, (theBytes == 0 && theBytes < aSize) ? aSize : theBytes);
}

StString StRawFile::readTextFile(const StCString& theFilePath) {
    StRawFile stTextFile(theFilePath);
    if(stTextFile.readFile()) {
        return stTextFile.getAsANSIText();
    }
    return StString();
}
