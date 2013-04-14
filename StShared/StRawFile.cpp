/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <cstdio> // forward-declared to work-around broken MinGW headers

#include <StFile/StRawFile.h>

#include <iostream>
#include <fstream>

StRawFile::StRawFile(const StString& theFilePath,
                     StNode* theParentNode)
: StFileNode(theFilePath, theParentNode),
  myFileHandle(NULL),
  myBuffer(NULL),
  myBuffSize(0) {
    //
}

StRawFile::~StRawFile() {
    closeFile();
    freeBuffer();
}

bool StRawFile::openFile(StRawFile::ReadWrite theFlags,
                         const StString& theFilePath) {
    // close previously opened file handle if any
    closeFile();

    if(!theFilePath.isEmpty()) {
        setSubPath(theFilePath);
    }

    StString aFilePath = getPath();
#if (defined(_WIN32) || defined(__WIN32__))
    myFileHandle = _wfopen(aFilePath.toUtfWide().toCString(), (theFlags == StRawFile::WRITE) ? L"wb" : L"rb");
#else
    myFileHandle =   fopen(aFilePath.toCString(),             (theFlags == StRawFile::WRITE) ?  "wb" :  "rb");
#endif
    return myFileHandle != NULL;
}

void StRawFile::closeFile() {
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
    myBuffSize = theDataSize;
    myBuffer = stMemAllocAligned<stUByte_t*>(myBuffSize + 1);
    myBuffer[myBuffSize] = '\0'; // just for safe
}

void StRawFile::freeBuffer() {
    stMemFreeAligned(myBuffer);
    myBuffer = NULL;
    myBuffSize = 0;
}

bool StRawFile::readFile(const StString& theFilePath) {
    freeBuffer();

    if(!openFile(StRawFile::READ, theFilePath)) {
        return false;
    }

    // determine length of file
    fseek(myFileHandle, 0, SEEK_END);
    long aFileLen = ftell(myFileHandle);
    if(aFileLen <= 0L) {
        closeFile();
        return false;
    }

    // create a buffer and read the data
    initBuffer(size_t(aFileLen));

    fseek(myFileHandle, 0, SEEK_SET);
    if(myBuffSize > 0) {
        size_t aCountRead = fread(myBuffer, 1, myBuffSize, myFileHandle);
        (void )aCountRead;
    }
    closeFile();
    return true;
}

bool StRawFile::saveFile(const StString& theFilePath) {
    if(!openFile(StRawFile::WRITE, theFilePath)) {
        return false;
    }
    bool isSuccess = writeFile() == myBuffSize;
    closeFile();
    return isSuccess;
}

size_t StRawFile::writeFile(size_t theBytes) {
    if(!isOpen() || myBuffSize == 0) {
        return 0;
    }
    return fwrite(myBuffer, 1, (theBytes == 0 && theBytes < myBuffSize) ? myBuffSize : theBytes, myFileHandle);
}

StString StRawFile::readTextFile(const StString& theFilePath) {
    StRawFile stTextFile(theFilePath);
    if(stTextFile.readFile()) {
        return stTextFile.getAsANSIText();
    }
    return StString();
}
