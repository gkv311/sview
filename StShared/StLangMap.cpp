/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StStrings/StLangMap.h>
#include <StStrings/StLogger.h>

#include <fstream> // file input/output

#if (defined(_WIN32) || defined(__WIN32__))
    #include <windows.h>
#endif

namespace {
    static const stUtf8_t HEADER_SECTION_DELIM[] = "--------";
    static const size_t READ_BUFFER_SIZE = 4096U;
};

void StLangMap::parseLine(const StString& theLine) {
    if(myIsHeaderSection) {
        myIsHeaderSection = !(theLine == StString(HEADER_SECTION_DELIM));
    }

    for(StUtf8Iter anIter = theLine.iterator(); *anIter != 0; ++anIter) {
        if(*anIter != stUtf32_t('=')) {
            // not interesting
            continue;
        }
        size_t aKey = size_t(std::atol(theLine.subString(0, anIter.getIndex()).toCString()));

        // get value without quotes
        StString aValue = theLine.subString(anIter.getIndex() + 2, theLine.getLength() - 1);

        ///TODO (Kirill Gavrilov#9) add all replacements
        for(anIter = aValue.iterator(); *anIter != 0; ++anIter) {
            if(*anIter.getBufferHere() == stUtf8_t('\\') && *anIter.getBufferNext() == stUtf8_t('n')) {
                // this is a hacking code in fact...
                *(stUtf8_t* )anIter.getBufferHere() = stUtf8_t(' ');
                *(stUtf8_t* )anIter.getBufferNext() = stUtf8_t('\n');
            }
        }
        myMap.insert(std::pair<size_t, StString>(aKey, aValue));
        return;
    }
}

StLangMap::StLangMap()
: myLngFile(),
  myMap(),
  myIsHeaderSection(true),
  myToShowId(true) {
    //
}

StLangMap::StLangMap(const StString& theLngFilePath)
: myLngFile(),
  myMap(),
  myIsHeaderSection(true),
  myToShowId(true) {
    open(theLngFilePath);
}

StLangMap::~StLangMap() {
    //
}

bool StLangMap::open(const StString& theLngFilePath) {
    myLngFile = theLngFilePath;

#ifdef _WIN32
    // it is possible to use std::ifstream, but only for ANSI filenames
    HANDLE inFile = CreateFileW(myLngFile.toUtfWide().toCString(), GENERIC_READ, FILE_SHARE_READ, NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(inFile == INVALID_HANDLE_VALUE) {
        ST_DEBUG_LOG("StLangMap, Failed to open language file \"" + myLngFile + '\"');
        return false;
    }
#else
    std::ifstream inFile;
    inFile.open(myLngFile.toCString());
    if(inFile.fail()) {
        ST_DEBUG_LOG("StLangMap, Failed to open language file \"" + myLngFile + '\"');
        return false;
    }
#endif
    char bufferOrig[READ_BUFFER_SIZE]; bufferOrig[0] = '\0';
    char* bufferLineOrig = new char[1];
    bufferLineOrig[0] = '\0';
    size_t aLineSize = 0;

    StString bufferLineUTF;
    bool isCont = false;
    size_t oldLen = 0;

#ifdef _WIN32
    DWORD aBytesRead = 0;
    while(ReadFile(inFile, bufferOrig, READ_BUFFER_SIZE, &aBytesRead, NULL)) {
        if(aBytesRead < 1) {
            break;
        }
#else
    while(!inFile.eof()) {
        inFile.read(bufferOrig, READ_BUFFER_SIZE);
        const size_t aBytesRead = inFile.gcount();
        if(aBytesRead < 1) {
            break;
        }
#endif
        size_t lineStart = 0;
        for(size_t c = 0; c < (size_t )aBytesRead; ++c) {
            if(bufferOrig[c] == '\n') {
                if(isCont) {
                    char* aCopy = new char[oldLen + c - lineStart + 1];
                    stMemCpy(&aCopy[0], bufferLineOrig, oldLen);
                    stMemCpy(&aCopy[oldLen], &bufferOrig[lineStart], (c - lineStart));
                    aLineSize = oldLen + c - lineStart;
                    delete[] bufferLineOrig;
                    bufferLineOrig = aCopy;
                } else {
                    delete[] bufferLineOrig;
                    bufferLineOrig = new char[c - lineStart + 1];
                    stMemCpy(bufferLineOrig, &bufferOrig[lineStart], (c - lineStart));
                    aLineSize = c - lineStart;
                }
                // remove CR symbol if needed
                if(aLineSize > 0 && bufferLineOrig[aLineSize - 1] == stUtf8_t(13)) {
                    --aLineSize;
                }
                bufferLineOrig[aLineSize] = '\0';

                bufferLineUTF = StString(bufferLineOrig);
                parseLine(bufferLineUTF);

                lineStart = c + 1;
                oldLen = 0;
                isCont = false;

            } else if(c == (READ_BUFFER_SIZE - 1)) {
                char* aCopy = new char[oldLen + READ_BUFFER_SIZE - lineStart];
                if(oldLen > 0) {
                    stMemCpy(aCopy, bufferLineOrig, oldLen);
                }
                stMemCpy(&aCopy[oldLen], &bufferOrig[lineStart], (READ_BUFFER_SIZE - lineStart));
                delete[] bufferLineOrig;
                bufferLineOrig = aCopy;
                oldLen += (READ_BUFFER_SIZE - lineStart);
                isCont = true;
            }

            if(!isCont) {
                delete[] bufferLineOrig;
                bufferLineOrig = new char[1];
                bufferLineOrig[0] = '\0';
            }
        }
    }
    delete[] bufferLineOrig;
#ifdef _WIN32
    CloseHandle(inFile);
#else
    inFile.close();
#endif
    ST_DEBUG_LOG("StLangMap, Loaded language file \"" + myLngFile + '\"');
    return true;
}

StString& StLangMap::changeValue(const size_t theId) {
    return myMap[theId];
}

StString& StLangMap::operator[](const size_t theId) {
    return myMap[theId];
}

StString& StLangMap::changeValueId(const size_t theId,
                                   const char*  theDefaultValue) {
    StString& aValue = myMap[theId];
    if(aValue.isEmpty()) {
        if(myToShowId) {
            aValue = StString('[') + theId + ']' + theDefaultValue;
        } else {
            aValue = theDefaultValue;
        }
    }
    return aValue;
}

StString& StLangMap::operator()(const size_t theId,
                                const char*  theDefaultValue) {
    return changeValueId(theId, theDefaultValue);
}

size_t StLangMap::size() const {
    return myMap.size();
}

void StLangMap::clear() {
    myMap.clear();
}
