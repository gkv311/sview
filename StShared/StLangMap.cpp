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

namespace {
    static const stUtf8_t HEADER_SECTION_DELIM[] = "--------";
    static const size_t READ_BUFFER_SIZE = 4096U;
}

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

StLangMap::~StLangMap() {
    //
}

bool StLangMap::read(const char* theContent,
                     const int   theLen) {
    if(theContent == NULL
    || theLen < 1) {
        return false;
    }

    char* bufferLineOrig = new char[1];
    bufferLineOrig[0] = '\0';
    size_t aLineSize = 0;

    StString bufferLineUTF;
    bool isCont = false;
    size_t oldLen = 0;

    size_t lineStart = 0;
    for(size_t c = 0; c < (size_t )theLen; ++c) {
        if(theContent[c] == '\n') {
            if(isCont) {
                char* aCopy = new char[oldLen + c - lineStart + 1];
                stMemCpy(&aCopy[0], bufferLineOrig, oldLen);
                stMemCpy(&aCopy[oldLen], &theContent[lineStart], (c - lineStart));
                aLineSize = oldLen + c - lineStart;
                delete[] bufferLineOrig;
                bufferLineOrig = aCopy;
            } else {
                delete[] bufferLineOrig;
                bufferLineOrig = new char[c - lineStart + 1];
                stMemCpy(bufferLineOrig, &theContent[lineStart], (c - lineStart));
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
            stMemCpy(&aCopy[oldLen], &theContent[lineStart], (READ_BUFFER_SIZE - lineStart));
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
    delete[] bufferLineOrig;
    return true;
}

StString& StLangMap::changeValue(const size_t theId) {
    return myMap[theId];
}

const StString& StLangMap::getValue(const size_t theId) const {
    const std::map<size_t, StString>::const_iterator anIter = myMap.find(theId);
    return (anIter != myMap.end()) ? anIter->second : myEmptyStr;
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

void StLangMap::addAlias(const StString& theStringKey,
                         const size_t    theIntKey) {
    myMapStrKeys[theStringKey] = theIntKey;
}

const StString& StLangMap::getValue(const StString& theStringKey) const {
    const std::map<StString, size_t>::const_iterator anIter = myMapStrKeys.find(theStringKey);
    return anIter != myMapStrKeys.end()
         ? getValue(anIter->second)
         : myEmptyStr;
}

size_t StLangMap::size() const {
    return myMap.size();
}

void StLangMap::clear() {
    myMap.clear();
}
