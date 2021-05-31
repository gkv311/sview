/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StStrings/StLangMap.h>
#include <StStrings/StLogger.h>

#include <fstream> // file input/output

namespace {
    static const StString ST_NEWLINE2            = "\\n";
    static const StString ST_NEWLINE_REPLACEMENT = " \x0A";

    /**
     * Auxiliary function to seek iterator to end of the current line.
     */
    inline void seekToEOL(StUtf8Iter& theIter,
                          const char* theEnd) {
        for(; theIter.getBufferHere() < theEnd && *theIter != 0; ++theIter) {
            if(*theIter == stUtf32_t('\n')) {
                return;
            }
        }
    }
}

StLangMap::StLangMap()
: myToShowId(true) {
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

    const char* anEnd = theContent + theLen;
    size_t aNbLines = 0;
    for(StUtf8Iter aCharIter(theContent); aCharIter.getBufferHere() < anEnd && *aCharIter != 0; ++aCharIter) {
        if(*aCharIter == '-'
        || *aCharIter == '#'
        || *aCharIter == ';'
        || *aCharIter == ' '
        || *aCharIter == '@') {
            seekToEOL(aCharIter, anEnd); // skip the comments
            ++aNbLines;
            continue;
        } else if(*aCharIter == stUtf32_t('\n')) {
            ++aNbLines;
            continue;
        }

        //bool isTemp = false;
        if(*aCharIter == stUtf32_t('?')) {
            ++aCharIter; // skip TODO flag
            //isTemp = true;
        }

        char* anEndPtr = NULL;
        const size_t aKey = (size_t )::strtol(aCharIter.getBufferHere(), &anEndPtr, 10);
        if(anEndPtr == aCharIter.getBufferHere()) {
            seekToEOL(aCharIter, anEnd);
            ++aNbLines;
            continue;
        }

        aCharIter.init(anEndPtr);
        if(*aCharIter != '=') {
            seekToEOL(aCharIter, anEnd);
            ++aNbLines;
            continue;
        }

        StUtf8Iter aValuePos = ++aCharIter;
        seekToEOL(aCharIter, anEnd);

        size_t aValueSize = aCharIter.getBufferHere() - aValuePos.getBufferHere();
        size_t aValueLen  = aCharIter.getIndex()      - aValuePos.getIndex();
        if(aValueLen != 0
        && *(aValuePos.getBufferHere() + aValueSize - 1) == char(13)) {
            --aValueLen;
            --aValueSize;
        }
        if(aValueLen >= 2
        && *aValuePos == '\"'
        && *(aValuePos.getBufferHere() + aValueSize - 1) == '\"') {
            ++aValuePos;
            aValueLen  -= 2;
            aValueSize -= 2;
        }
        StString aValue(aValuePos.getBufferHere(), aValueLen);
        aValue.replaceFast(ST_NEWLINE2, ST_NEWLINE_REPLACEMENT);
        myMap.insert(std::pair<size_t, StString>(aKey, aValue));
    }
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
