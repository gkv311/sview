/**
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StStringUnicode_inl__
#define __StStringUnicode_inl__

/**
 * Compute advance for specified string.
 */
template<typename TypeTo> template<typename TypeFrom> inline
void StStringUnicode<TypeTo>::strGetAdvance(const TypeFrom* theStringUtf,
                                            const size_t    theLengthMax,
                                            size_t&         theSizeBytes,
                                            size_t&         theLength) {
    theSizeBytes = 0;
    theLength = 0;
    StUtfIterator<TypeFrom> anIter(theStringUtf);
    switch(sizeof(TypeTo)) {
        case sizeof(stUtf8_t): {
            for(; *anIter != 0 && anIter.getIndex() < theLengthMax; ++anIter) {
                theSizeBytes += anIter.getAdvanceBytesUtf8();
            }
            theLength = anIter.getIndex();
            return;
        }
        case sizeof(stUtf16_t): {
            for(; *anIter != 0 && anIter.getIndex() < theLengthMax; ++anIter) {
                theSizeBytes += anIter.getAdvanceBytesUtf16();
            }
            theLength = anIter.getIndex();
            return;
        }
        case sizeof(stUtf32_t): {
            for(; *anIter != 0 && anIter.getIndex() < theLengthMax; ++anIter) {
                theSizeBytes += anIter.getAdvanceBytesUtf32();
            }
            theLength = anIter.getIndex();
            return;
        }
        default: return;
    }
}

template<typename Type>
stUtf32_t StStringUnicode<Type>::getChar(const size_t theCharIndex) const {
    ST_DEBUG_ASSERT(theCharIndex < myLength);
    StUtfIterator<Type> anIter(myString);
    for(; *anIter != 0; ++anIter) {
        if(anIter.getIndex() == theCharIndex) {
            return *anIter;
        }
    }
    return 0;
}

template<typename Type>
const Type* StStringUnicode<Type>::getCharBuffer(const size_t theCharIndex) const {
    ST_DEBUG_ASSERT(theCharIndex < myLength);
    StUtfIterator<Type> anIter(myString);
    for(; *anIter != 0; ++anIter) {
        if(anIter.getIndex() == theCharIndex) {
            return anIter.getBufferHere();
        }
    }
    return NULL;
}

template<typename Type> inline
void StStringUnicode<Type>::clear() {
    stStrFree(myString);
    mySize   = 0;
    myLength = 0;
    myString = stStrAlloc(mySize);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode()
: myString(stStrAlloc(0)),
  mySize(0),
  myLength(0) {
    //
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const StStringUnicode& theCopy)
: myString(stStrAlloc(theCopy.mySize)),
  mySize(theCopy.mySize),
  myLength(theCopy.myLength) {
    stStrCopy((stUByte_t* )myString, (const stUByte_t* )theCopy.myString, mySize);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const StConstStringUnicode<Type>& theCopy)
: myString(stStrAlloc(theCopy.Size)),
  mySize(theCopy.Size),
  myLength(theCopy.Length) {
    stStrCopy((stUByte_t* )myString, (const stUByte_t* )theCopy.String, mySize);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const char*  theCopyUtf8,
                                       const size_t theLength)
: myString(NULL),
  mySize(0),
  myLength(0) {
    fromUnicode(theCopyUtf8, theLength);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const stUtf16_t* theCopyUtf16,
                                       const size_t     theLength)
: myString(NULL),
  mySize(0),
  myLength(0) {
    fromUnicode(theCopyUtf16, theLength);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const stUtf32_t* theCopyUtf32,
                                       const size_t     theLength)
: myString(NULL),
  mySize(0),
  myLength(0) {
    fromUnicode(theCopyUtf32, theLength);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const stUtfWide_t* theCopyUtfWide,
                                       const size_t       theLength)
: myString(NULL),
  mySize(0),
  myLength(0) {
    fromUnicode(theCopyUtfWide, theLength);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const char theChar)
: myString(NULL),
  mySize(0),
  myLength(0) {
    if(theChar == '\0') {
        // empty string
        mySize   = 0;
        myLength = 0;
        myString = stStrAlloc(mySize);
        return;
    }
    mySize   = sizeof(Type);
    myLength = 1;
    myString = stStrAlloc(mySize);
    myString[0] = Type(theChar);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const int32_t theInt32)
: myString(NULL),
  mySize(0),
  myLength(0) {
    char aBuff[16];
    stsprintf(aBuff, 16, "%d", theInt32);
    fromUnicode(aBuff);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const uint32_t theUInt32)
: myString(NULL),
  mySize(0),
  myLength(0) {
    char aBuff[16];
    stsprintf(aBuff, 16, "%u", theUInt32);
    fromUnicode(aBuff);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const int64_t theInt64)
: myString(NULL),
  mySize(0),
  myLength(0) {
    char aBuff[32];
#if defined(_MSC_VER)
    stsprintf(aBuff, 32, "%I64i", theInt64);
#elif (defined(_WIN64) || defined(__WIN64__)\
   ||  defined(_LP64)  || defined(__LP64__))
    stsprintf(aBuff, 32, "%li",   theInt64);
#else
    stsprintf(aBuff, 32, "%lli",  theInt64);
#endif
    fromUnicode(aBuff);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const uint64_t theUInt64)
: myString(NULL),
  mySize(0),
  myLength(0) {
    char aBuff[32];
#if defined(_MSC_VER)
    stsprintf(aBuff, 32, "%I64u", theUInt64);
#elif (defined(_WIN64) || defined(__WIN64__)\
   ||  defined(_LP64)  || defined(__LP64__))
    stsprintf(aBuff, 32, "%lu",   theUInt64);
#else
    stsprintf(aBuff, 32, "%llu",  theUInt64);
#endif
    fromUnicode(aBuff);
}

#ifdef ST_HAS_INT64_EXT
template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const stInt64ext_t theInt64)
: myString(NULL),
  mySize(0),
  myLength(0) {
    char aBuff[32];
#if defined(_MSC_VER)
    stsprintf(aBuff, 32, "%I64i", theInt64);
#elif (defined(_WIN64) || defined(__WIN64__)\
   ||  defined(_LP64)  || defined(__LP64__))
    stsprintf(aBuff, 32, "%li",   theInt64);
#else
    stsprintf(aBuff, 32, "%lli",  theInt64);
#endif
    fromUnicode(aBuff);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const stUInt64ext_t theUInt64)
: myString(NULL),
  mySize(0),
  myLength(0) {
    char aBuff[32];
#if defined(_MSC_VER)
    stsprintf(aBuff, 32, "%I64u", theUInt64);
#elif (defined(_WIN64) || defined(__WIN64__)\
   ||  defined(_LP64)  || defined(__LP64__))
    stsprintf(aBuff, 32, "%lu",   theUInt64);
#else
    stsprintf(aBuff, 32, "%llu",  theUInt64);
#endif
    fromUnicode(aBuff);
}
#endif

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const double theFloat)
: myString(NULL),
  mySize(0),
  myLength(0) {
    char aBuff[256];
    stsprintf(aBuff, 256, "%f", theFloat);
    fromUnicode(aBuff);
}

template<typename Type> inline
StStringUnicode<Type>::~StStringUnicode() {
    stStrFree(myString);
}

template<typename Type> inline
const StStringUnicode<Type>& StStringUnicode<Type>::operator=(const StStringUnicode<Type>& theOther) {
    if(this == &theOther) {
        return (*this);
    }
    stStrFree(myString);
    mySize   = theOther.mySize;
    myLength = theOther.myLength;
    myString = stStrAlloc(mySize);
    stStrCopy((stUByte_t* )myString, (const stUByte_t* )theOther.myString, mySize);
    return (*this);
}

template<typename Type> template<typename TypeFrom>
void StStringUnicode<Type>::fromUnicode(const TypeFrom* theStringUtf,
                                        const size_t    theLength) {
    Type* anOldBuffer = myString; // necessary in case of self-copying
    StUtfIterator<TypeFrom> anIterRead(theStringUtf);
    if(*anIterRead == 0) {
        // special case
        clear();
        return;
    }

    switch(sizeof(TypeFrom)) { // use switch() rather than if() to shut up msvc compiler
        case sizeof(Type): {
            if(theLength != size_t(-1)) {
                // optimized copy
                for(; *anIterRead != 0 && anIterRead.getIndex() < theLength; ++anIterRead) {}

                mySize   = size_t((stUByte_t* )anIterRead.getBufferHere() - (stUByte_t* )theStringUtf);
                myLength = anIterRead.getIndex();
                myString = stStrAlloc(mySize);
                stStrCopy((stUByte_t* )myString, (const stUByte_t* )theStringUtf, mySize);
                stStrFree(anOldBuffer);
                return;
            }
        }
        default: break;
    }

    strGetAdvance(theStringUtf, theLength, mySize, myLength);
    myString = stStrAlloc(mySize);
    // reset iterator
    anIterRead.init(theStringUtf);
    Type* anIterWrite = myString;
    for(; *anIterRead != 0 && anIterRead.getIndex() < theLength; ++anIterRead) {
        anIterWrite = anIterRead.getUtf(anIterWrite);
    }
    stStrFree(anOldBuffer);
}

template<typename Type> inline
void StStringUnicode<Type>::fromLocale(const char*  theString,
                                       const size_t theLength) {
#if(defined(_WIN32) || defined(__WIN32__))
    // use WinAPI
    int aWideSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, theString, -1, NULL, 0);
    if(aWideSize <= 0) {
        clear();
        return;
    }
    wchar_t* aWideBuffer = new wchar_t[aWideSize + 1];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, theString, -1, aWideBuffer, aWideSize);
    aWideBuffer[aWideSize] = L'\0';
    fromUnicode(aWideBuffer, theLength);
    delete[] aWideBuffer;
#else
    // this is size in bytes but should probably be enough to store string in wide chars
    // notice that these functions are sensitive to locale set by application!
    int aMbLen = mblen(theString, MB_CUR_MAX);
    if(aMbLen <= 0) {
        clear();
        return;
    }
    wchar_t* aWideBuffer = new wchar_t[aMbLen + 1];
    mbstowcs(aWideBuffer, theString, aMbLen);
    aWideBuffer[aMbLen] = L'\0';
    fromUnicode(aWideBuffer, theLength);
    delete[] aWideBuffer;
#endif
}

template<typename Type> inline
bool StStringUnicode<Type>::toLocale(char*     theBuffer,
                                     const int theSizeBytes) const {
    StStringUnicode<wchar_t> aWideCopy(myString, myLength);
#if(defined(_WIN32) || defined(__WIN32__))
    int aMbBytes = WideCharToMultiByte(CP_ACP, 0, aWideCopy.toCString(), -1, theBuffer, theSizeBytes, NULL, NULL);
#else
    int aMbBytes = wcstombs(theBuffer, aWideCopy.toCString(), theSizeBytes);
#endif
    if(aMbBytes <= 0) {
        *theBuffer = '\0';
        return false;
    }
    return true;
}

template<typename Type> inline
const StStringUnicode<Type>& StStringUnicode<Type>::operator=(const char* theStringUtf8) {
    fromUnicode(theStringUtf8);
    return (*this);
}

template<typename Type> inline
const StStringUnicode<Type>& StStringUnicode<Type>::operator=(const stUtfWide_t* theStringUtfWide) {
    fromUnicode(theStringUtfWide);
    return (*this);
}

template<typename Type> inline
bool StStringUnicode<Type>::isEquals(const StStringUnicode& theCompare) const {
    return this == &theCompare
        || stStrAreEqual(myString, mySize, theCompare.myString, theCompare.mySize);
}

// TODO (Kirill Gavrilov#9) case ignored only for ANSI symbols
template<typename Type> inline
bool StStringUnicode<Type>::isEqualsIgnoreCase(const StStringUnicode& theCompare) const {
    if(this == &theCompare) {
        return true;
    } else if(mySize != theCompare.mySize) {
        return false;
    }
    StUtfIterator<Type> anIter1(myString);
    StUtfIterator<Type> anIter2(theCompare.myString);
    for(;; ++anIter1, ++anIter2) {
        if( *anIter2 > 64 && *anIter2 < 91         // ANSI big case
        && (*anIter2 == *anIter1 || (*anIter2 + 32) == *anIter1)) {
            continue;
        } else if( *anIter2 > 96 && *anIter2 < 123 // ANSI little case
               && (*anIter2 == *anIter1 || (*anIter2 - 32) == *anIter1)) {
            continue;
        } else if(*anIter2 != *anIter1) {
            return false;
        } else if(*anIter1 == 0) {
            return true;
        }
    }
}

template<typename Type> inline
bool StStringUnicode<Type>::operator!=(const StStringUnicode& theCompare) const {
    return (!StStringUnicode::operator==(theCompare));
}

template<typename Type> inline
bool StStringUnicode<Type>::operator>(const StStringUnicode& theCompare) const {
    if(&theCompare == this) {
        return false;
    }
    StUtfIterator<Type> anIterMe(myString);
    StUtfIterator<Type> anIterOther(theCompare.myString);
    for(;; ++anIterMe, ++anIterOther) {
        if(*anIterMe == 0) {
            return false;
        } else if(*anIterOther == 0) {
            return true;
        } else if(*anIterMe == *anIterOther) {
            continue;
        } else {
            return *anIterMe > *anIterOther;
        }
    }
}

template<typename Type> inline
bool StStringUnicode<Type>::operator<(const StStringUnicode& theCompare) const {
    if(&theCompare == this) {
        return false;
    }
    StUtfIterator<Type> anIterMe(myString);
    StUtfIterator<Type> anIterOther(theCompare.myString);
    for(;; ++anIterMe, ++anIterOther) {
        if(*anIterMe == 0) {
            return true;
        } else if(*anIterOther == 0) {
            return false;
        } else if(*anIterMe == *anIterOther) {
            continue;
        } else {
            return *anIterMe < *anIterOther;
        }
    }
}

template<typename Type> inline
bool StStringUnicode<Type>::operator>=(const StStringUnicode& theCompare) const {
    if(&theCompare == this) {
        return true;
    }
    StUtfIterator<Type> anIterMe(myString);
    StUtfIterator<Type> anIterOther(theCompare.myString);
    bool isLastMe, isLastOther;
    for(;; ++anIterMe, ++anIterOther) {
        isLastMe    = (*anIterMe    == 0);
        isLastOther = (*anIterOther == 0);
        if(isLastMe && isLastOther) {
            return true;
        } else if(isLastMe) {
            return false;
        } else if(isLastOther) {
            return true;
        } else if(*anIterMe == *anIterOther) {
            continue;
        } else {
            return *anIterMe > *anIterOther;
        }
    }
}

template<typename Type> inline
bool StStringUnicode<Type>::operator<=(const StStringUnicode& theCompare) const {
    if(&theCompare == this) {
        return true;
    }
    StUtfIterator<Type> anIterMe(myString);
    StUtfIterator<Type> anIterOther(theCompare.myString);
    bool isLastMe, isLastOther;
    for(;; ++anIterMe, ++anIterOther) {
        isLastMe    = (*anIterMe    == 0);
        isLastOther = (*anIterOther == 0);
        if(isLastMe && isLastOther) {
            return true;
        } else if(isLastMe) {
///TODO (Kirill Gavrilov#1) check conditions
            return false;
        } else if(isLastOther) {
            return true;
        } else if(*anIterMe == *anIterOther) {
            continue;
        } else {
            return *anIterMe < *anIterOther;
        }
    }
}

template<typename Type> inline
StStringUnicode<Type>& StStringUnicode<Type>::operator+=(const StStringUnicode<Type>& theAppend) {
    if(theAppend.isEmpty()) {
        return (*this);
    }
    // create new string
    size_t aSize = mySize + theAppend.mySize;
    Type* aString = stStrAlloc(aSize);
    stStrCopy((stUByte_t* )aString,          (const stUByte_t* )myString,           mySize);
    stStrCopy((stUByte_t* )aString + mySize, (const stUByte_t* )theAppend.myString, theAppend.mySize);

    stStrFree(myString);
    mySize   = aSize;
    myString = aString;
    myLength += theAppend.myLength;
    return (*this);
}

template<typename Type> inline
StStringUnicode<Type> StStringUnicode<Type>::subString(const size_t theStart,
                                                       const size_t theEnd) const {
    if(theStart >= theEnd) {
        return StStringUnicode<Type>();
    }
    for(StUtfIterator<Type> anIter(myString); *anIter != 0; ++anIter) {
        if(anIter.getIndex() >= theStart) {
            return StStringUnicode<Type>(anIter.getBufferHere(), theEnd - theStart);
        }
    }
    return StStringUnicode<Type>();
}

template<typename Type> inline
const StStringUnicode<stUtf8_t> StStringUnicode<Type>::toUtf8() const {
    StStringUnicode<stUtf8_t> aCopy;
    aCopy.fromUnicode(myString);
    return aCopy;
}

template<typename Type> inline
const StStringUnicode<stUtf16_t> StStringUnicode<Type>::toUtf16() const {
    StStringUnicode<stUtf16_t> aCopy;
    aCopy.fromUnicode(myString);
    return aCopy;
}

template<typename Type> inline
const StStringUnicode<stUtf32_t> StStringUnicode<Type>::toUtf32() const {
    StStringUnicode<stUtf32_t> aCopy;
    aCopy.fromUnicode(myString);
    return aCopy;
}

template<typename Type> inline
const StStringUnicode<stUtfWide_t> StStringUnicode<Type>::toUtfWide() const {
    StStringUnicode<stUtfWide_t> aCopy;
    aCopy.fromUnicode(myString);
    return aCopy;
}

template<typename Type> inline
const StStringUnicode<stUtf8_t> StStringUnicode<Type>::toDump() const {
    StStringUnicode<stUtf8_t> aDump;
    aDump += StStringUnicode<stUtf8_t>("StStringUnicode<") + sizeof(Type) + ">, size= " + mySize + ", length= " + myLength + ", text= '";
    switch(sizeof(Type)) {
        case sizeof(stUtf8_t): {
            aDump += (const stUtf8_t* )myString;
            break;
        }
        default: {
            aDump += toUtf8();
            break;
        }
    }
    return aDump + "'";
}

template<typename Type> inline
bool StStringUnicode<Type>::isStartsWith(const StStringUnicode<Type>& theStartString) const {
    return (this == &theStartString)
        || subString(0, theStartString.getLength()).isEquals(theStartString);
}

template<typename Type> inline
bool StStringUnicode<Type>::isStartsWithIgnoreCase(const StStringUnicode<Type>& theStartString) const {
    return (this == &theStartString)
        || subString(0, theStartString.getLength()).isEqualsIgnoreCase(theStartString);
}

template<typename Type> inline
bool StStringUnicode<Type>::isEndsWith(const StStringUnicode<Type>& theEndString) const {
    if(this == &theEndString) {
        return true;
    }
    return (myLength >= theEndString.myLength)
        && subString(myLength - theEndString.myLength, myLength).isEquals(theEndString);
}

template<typename Type> inline
bool StStringUnicode<Type>::isEndsWithIgnoreCase(const StStringUnicode<Type>& theEndString) const {
    if(this == &theEndString) {
        return true;
    }
    return (myLength >= theEndString.myLength)
        && subString(myLength - theEndString.myLength, myLength).isEqualsIgnoreCase(theEndString);
}

template<typename Type> inline
StHandle <StArrayList< StStringUnicode<Type> > > StStringUnicode<Type>::split(const stUtf32_t theDelimeter,
                                                                              const size_t    theLimitNb) const {
    StHandle< StArrayList< StStringUnicode<Type> > > aSplitList
        = new StArrayList< StStringUnicode<Type> >((theLimitNb < size_t(-1)) ? theLimitNb : 16);

    size_t aStart = 0;
    size_t aSplitCount = 1; // set to 1, that is
    for(StUtfIterator<Type> anIter(myString);; ++anIter) {
        if(*anIter == 0) {
            StStringUnicode<Type> aSplit = subString(aStart, anIter.getIndex());
            if(!aSplit.isEmpty()) {
                aSplitList->add(aSplit);
            }
            break;
        } else if(*anIter == theDelimeter) {
            aSplitList->add(subString(aStart, anIter.getIndex()));
            aStart = anIter.getIndex() + 1;
            if(++aSplitCount >= theLimitNb) {
                if(myLength > aStart) {
                    aSplitList->add(subString(aStart, myLength));
                }
                break;
            }
        }
    }
    return aSplitList;
}

template<typename Type> inline
bool StStringUnicode<Type>::isContains(const stUtf32_t theSubChar) const {
    for(StUtfIterator<Type> anIter(myString); *anIter != 0; ++anIter) {
        if(stUtf32_t(*anIter) == theSubChar) {
            return true;
        }
    }
    return false;
}

template<typename Type> inline
bool StStringUnicode<Type>::isContains(const StStringUnicode<Type>& theSubString) const {
    if(theSubString.isEmpty()) {
        return true;
    }
    StUtfIterator<Type> anIterMe(myString);
    StUtfIterator<Type> anIterSub(theSubString.myString);
    for(;; ++anIterMe) {
        if(*anIterMe == 0) {
            return *anIterSub == 0;
        } else if(*anIterSub == 0) {
            return true;
        } else if(*anIterMe == *anIterSub) {
            ++anIterSub;
        } else {
            anIterSub.init(theSubString.myString);
        }
    }
}

template<typename Type> inline
StStringUnicode<Type> StStringUnicode<Type>::unquoted() const {
    if(myLength < 2) {
        return *this;
    }
    const Type* aLastSymbol = getCharBuffer(myLength - 1);
    if((aLastSymbol != myString)
    && ((*myString == Type('\"') && *aLastSymbol == Type('\"'))
     || (*myString == Type('\'') && *aLastSymbol == Type('\'')))) {
        return subString(1, myLength - 1);
    }
    return *this;
}

template<typename Type> inline
StStringUnicode<Type> StStringUnicode<Type>::replace(const StStringUnicode<Type>& theSubString,
                                                     const StStringUnicode<Type>& theReplacer) const {
    if(theSubString.isEmpty() || isEmpty() || theSubString.mySize >= mySize) {
        // just make a copy
        return *this;
    }
    StUtfIterator<Type> anIter(myString);
    StStringUnicode<Type> aResult;
    size_t aStart = 0;
    for(size_t aCharId = 0; *anIter != 0;) {
        if(stAreEqual(anIter.getBufferHere(), theSubString.myString, theSubString.mySize)) {
            aResult += subString(aStart, aCharId);
            aResult += theReplacer;
            aStart  = aCharId + theSubString.myLength;
            aCharId = aStart;
            anIter.init(anIter.getBufferHere() + theSubString.mySize);
            continue;
        }
        ++anIter;
        ++aCharId;
    }
    if(aStart < myLength) {
        aResult += subString(aStart, myLength);
    }
    return aResult;
}

template<typename Type> inline
void StStringUnicode<Type>::replaceFast(const StStringUnicode<Type>& theSubString,
                                        const StStringUnicode<Type>& theReplacer) {
    if(theSubString.isEmpty() || isEmpty() || theSubString.mySize >= mySize) {
        // just make a copy
        return;
    } else if(theSubString.mySize != theReplacer.mySize) {
        // invalid method usage
        ST_DEBUG_ASSERT(theSubString.mySize == theReplacer.mySize);
        ///*this = replace(theSubString, theReplacer);
        return;
    }
    for(StUtfIterator<Type> anIter(myString); *anIter != 0;) {
        if(stAreEqual(anIter.getBufferHere(), theSubString.myString, theSubString.mySize)) {
            stStrCopy((stUByte_t* )anIter.changeBufferHere(), (const stUByte_t* )theReplacer.myString, theReplacer.mySize);
            anIter.init(anIter.getBufferHere() + theSubString.mySize);
            continue;
        }
        ++anIter;
    }
}

template<typename Type> inline
int StStringUnicode<Type>::hexPairValue(const Type* theCode) {
    int aValue = 0;
    int aDigit;
    for(const Type* anIter = theCode;;) {
        aDigit = *anIter++;
        if(aDigit >= '0' && aDigit <= '9') {
            aValue += aDigit - '0';
        } else if (aDigit >= 'A' && aDigit <= 'F') {
            aValue += aDigit - 'A' + 10;
        } else if (aDigit >= 'a' && aDigit <= 'f') {
            aValue += aDigit - 'a' + 10;
        } else {
            return -1;
        }
        if(anIter == theCode + 2) {
            return aValue;
        }
        aValue <<= 4;
    }
}

template<typename Type> inline
size_t StStringUnicode<Type>::urlDecode(const Type* theSrcUrl,
                                        stUtf8_t*   theOut) {
    const stUtf8_t* aStart = theOut;
    for(StUtfIterator<Type> anIter(theSrcUrl); *anIter != 0; ++anIter) {
        switch(*anIter) {
            /*case stUtf32_t('+'):
                // old-style space-symbol replacement (in current standard %20 = ' ')
                *(theOut++) = ' ';
                break;*/
            case stUtf32_t('%'):
                if(anIter.getBufferHere()[1] != Type('\0')
                && anIter.getBufferHere()[2] != Type('\0')) {
                    // coded symbol: %XX
                    int aValue = hexPairValue(anIter.getBufferHere() + 1);
                    if(aValue >= 0) {
                        *(theOut++) = (stUtf8_t )aValue;
                        ++anIter;
                        ++anIter;
                    } else {
                        *theOut++ = '?';
                    }
                } else {
                    *theOut++ = '?';
                }
                break;
            default:
                theOut = anIter.getUtf(theOut);
        }
    }
    // remove trailing CR symbol
    if(theOut > aStart && *--theOut != stUtf8_t(13)) {
        ++theOut;
    }
    *theOut = '\0';
    return theOut - aStart;
}

/**
 * Define function for standard output (UTF-8 string).
 */
inline std::ostream& operator<<(std::ostream& theStream,
                                const StStringUnicode<stUtf8_t>& theString) {
    theStream << theString.toCString();
    return theStream;
}

/**
 * Define function for standard output (UTF-16 string).
 */
inline std::ostream& operator<<(std::ostream& theStream,
                                const StStringUnicode<stUtf16_t>& theString) {
    theStream << theString.toUtf8();
    return theStream;
}

/**
 * Define function for standard output (UTF-32 string).
 */
inline std::ostream& operator<<(std::ostream& theStream,
                                const StStringUnicode<stUtf32_t>& theString) {
    theStream << theString.toUtf8();
    return theStream;
}

#if(defined(_WIN32) || defined(__WIN32__))
/**
 * Define function for wide standard output.
 */
inline std::wostream& operator<<(std::wostream& theStream,
                                 const StStringUnicode<stUtfWide_t>& theString) {
    theStream << theString.toCString();
    return theStream;
}

/**
 * Define function for wide standard output (UTF-8 string).
 */
inline std::wostream& operator<<(std::wostream& theStream,
                                 const StStringUnicode<stUtf8_t>& theString) {
    theStream << theString.toUtfWide();
    return theStream;
}

/**
 * Define function for wide standard output (UTF-16 string).
 */
inline std::wostream& operator<<(std::wostream& theStream,
                                 const StStringUnicode<stUtf16_t>& theString) {
    theStream << theString.toUtfWide();
    return theStream;
}

/**
 * Define function for wide standard output (UTF-32 string).
 */
inline std::wostream& operator<<(std::wostream& theStream,
                                 const StStringUnicode<stUtf32_t>& theString) {
    theStream << theString.toUtfWide();
    return theStream;
}
#endif

#endif //__StStringUnicode_inl__
