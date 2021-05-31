/**
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
stUtf32_t StConstStringUnicode<Type>::getChar(const size_t theCharIndex) const {
    ST_DEBUG_ASSERT(theCharIndex < this->Length);
    StUtfIterator<Type> anIter(this->String);
    for(; *anIter != 0; ++anIter) {
        if(anIter.getIndex() == theCharIndex) {
            return *anIter;
        }
    }
    return 0;
}

template<typename Type>
const Type* StStringUnicode<Type>::getCharBuffer(const size_t theCharIndex) const {
    ST_DEBUG_ASSERT(theCharIndex < this->Length);
    StUtfIterator<Type> anIter(this->String);
    for(; *anIter != 0; ++anIter) {
        if(anIter.getIndex() == theCharIndex) {
            return anIter.getBufferHere();
        }
    }
    return NULL;
}

template<typename Type> inline
void StStringUnicode<Type>::clear() {
    stStrFree(this->String);
    this->Size   = 0;
    this->Length = 0;
    this->String = stStrAlloc(this->Size);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode() {
    this->String = stStrAlloc(0);
    this->Size   = 0;
    this->Length = 0;
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const StStringUnicode& theCopy) {
    this->String = stStrAlloc(theCopy.Size);
    this->Size   = theCopy.Size;
    this->Length = theCopy.Length;
    stStrCopy((stUByte_t* )this->String, (const stUByte_t* )theCopy.String, this->Size);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const StConstStringUnicode<Type>& theCopy) {
    this->String = stStrAlloc(theCopy.Size);
    this->Size   = theCopy.Size;
    this->Length = theCopy.Length;
    stStrCopy((stUByte_t* )this->String, (const stUByte_t* )theCopy.String, this->Size);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const char*  theCopyUtf8,
                                       const size_t theLength) {
    this->String = NULL;
    this->Size   = 0;
    this->Length = 0;
    fromUnicode(theCopyUtf8, theLength);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const stUtf16_t* theCopyUtf16,
                                       const size_t     theLength) {
    this->String = NULL;
    this->Size   = 0;
    this->Length = 0;
    fromUnicode(theCopyUtf16, theLength);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const stUtf32_t* theCopyUtf32,
                                       const size_t     theLength) {
    this->String = NULL;
    this->Size   = 0;
    this->Length = 0;
    fromUnicode(theCopyUtf32, theLength);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const stUtfWide_t* theCopyUtfWide,
                                       const size_t       theLength) {
    this->String = NULL;
    this->Size   = 0;
    this->Length = 0;
    fromUnicode(theCopyUtfWide, theLength);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const char theChar) {
    this->String = NULL;
    this->Size   = 0;
    this->Length = 0;
    if(theChar == '\0') {
        // empty string
        this->Size   = 0;
        this->Length = 0;
        this->String = stStrAlloc(this->Size);
        return;
    }
    this->Size   = sizeof(Type);
    this->Length = 1;
    this->String = stStrAlloc(this->Size);
    ((Type* )this->String)[0] = Type(theChar);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const int32_t theInt32) {
    this->String = NULL;
    this->Size   = 0;
    this->Length = 0;
    char aBuff[16];
    stsprintf(aBuff, 16, "%d", theInt32);
    fromUnicode(aBuff);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const uint32_t theUInt32) {
    this->String = NULL;
    this->Size   = 0;
    this->Length = 0;
    char aBuff[16];
    stsprintf(aBuff, 16, "%u", theUInt32);
    fromUnicode(aBuff);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const int64_t theInt64) {
    this->String = NULL;
    this->Size   = 0;
    this->Length = 0;
    char aBuff[32];
    stsprintf(aBuff, 32, "%" PRId64, theInt64);
    fromUnicode(aBuff);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const uint64_t theUInt64) {
    this->String = NULL;
    this->Size   = 0;
    this->Length = 0;
    char aBuff[32];
    stsprintf(aBuff, 32, "%" PRIu64, theUInt64);
    fromUnicode(aBuff);
}

#ifdef ST_HAS_INT64_EXT
template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const stInt64ext_t theInt64) {
    this->String = NULL;
    this->Size   = 0;
    this->Length = 0;
    char aBuff[32];
    stsprintf(aBuff, 32, "%" PRId64,  (int64_t )theInt64);
    fromUnicode(aBuff);
}

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const stUInt64ext_t theUInt64) {
    this->String = NULL;
    this->Size   = 0;
    this->Length = 0;
    char aBuff[32];
    stsprintf(aBuff, 32, "%" PRIu64, (uint64_t )theUInt64);
    fromUnicode(aBuff);
}
#endif

template<typename Type> inline
StStringUnicode<Type>::StStringUnicode(const double theFloat) {
    this->String = NULL;
    this->Size   = 0;
    this->Length = 0;
    char aBuff[256];
    stsprintf(aBuff, 256, "%f", theFloat);
    fromUnicode(aBuff);
}

template<typename Type> inline
StStringUnicode<Type>::~StStringUnicode() {
    stStrFree(this->String);
}

template<typename Type> inline
const StStringUnicode<Type>& StStringUnicode<Type>::operator=(const StStringUnicode<Type>& theOther) {
    if(this == &theOther) {
        return (*this);
    }
    stStrFree(this->String);
    this->Size   = theOther.Size;
    this->Length = theOther.Length;
    this->String = stStrAlloc(this->Size);
    stStrCopy((stUByte_t* )this->String, (const stUByte_t* )theOther.String, this->Size);
    return (*this);
}

template<typename Type> template<typename TypeFrom>
void StStringUnicode<Type>::fromUnicode(const StConstStringUnicode<TypeFrom>& theString) {
    fromUnicode(theString.toCString());
}

template<typename Type> template<typename TypeFrom>
void StStringUnicode<Type>::fromUnicode(const TypeFrom* theStringUtf,
                                        const size_t    theLength) {
    const Type* anOldBuffer = this->String; // necessary in case of self-copying
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

                this->Size   = size_t((stUByte_t* )anIterRead.getBufferHere() - (stUByte_t* )theStringUtf);
                this->Length = anIterRead.getIndex();
                this->String = stStrAlloc(this->Size);
                stStrCopy((stUByte_t* )this->String, (const stUByte_t* )theStringUtf, this->Size);
                stStrFree(anOldBuffer);
                return;
            }
        }
        default: break;
    }

    strGetAdvance(theStringUtf, theLength, this->Size, this->Length);
    this->String = stStrAlloc(this->Size);
    // reset iterator
    anIterRead.init(theStringUtf);
    const Type* anIterWrite = this->String;
    for(; *anIterRead != 0 && anIterRead.getIndex() < theLength; ++anIterRead) {
        anIterWrite = anIterRead.getUtf(anIterWrite);
    }
    stStrFree(anOldBuffer);
}

template<typename Type> inline
void StStringUnicode<Type>::fromLocale(const char*  theString,
                                       const size_t theLength) {
#ifdef _WIN32
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
    StStringUnicode<wchar_t> aWideCopy(this->String, this->Length);
#ifdef _WIN32
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
bool StConstStringUnicode<Type>::isEquals(const StConstStringUnicode& theCompare) const {
    return this == &theCompare
        || stStrAreEqual(this->String, this->Size, theCompare.String, theCompare.Size);
}

namespace {
// TODO (Kirill Gavrilov#9) case ignored only for ANSI symbols
template<typename Type> inline
bool isEqualsIgnoreCase(const StConstStringUnicode<Type>& theStr1,
                        const StConstStringUnicode<Type>& theStr2) {
    if(&theStr1 == &theStr2) {
        return true;
    } else if(theStr1.Size != theStr2.Size) {
        return false;
    }
    StUtfIterator<Type> anIter1(theStr1.String);
    StUtfIterator<Type> anIter2(theStr2.String);
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
};

template<typename Type> inline
bool StConstStringUnicode<Type>::isEqualsIgnoreCase(const StConstStringUnicode& theCompare) const {
    return ::isEqualsIgnoreCase(*this, theCompare);
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
    StUtfIterator<Type> anIterMe(this->String);
    StUtfIterator<Type> anIterOther(theCompare.String);
    for(;; ++anIterMe, ++anIterOther) {
        if(*anIterMe == 0) {
            return *anIterOther != 0;
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
    StUtfIterator<Type> anIterMe(this->String);
    StUtfIterator<Type> anIterOther(theCompare.String);
    for(;; ++anIterMe, ++anIterOther) {
        if(*anIterMe == 0) {
            return *anIterOther != 0;
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
    StUtfIterator<Type> anIterMe(this->String);
    StUtfIterator<Type> anIterOther(theCompare.String);
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
    StUtfIterator<Type> anIterMe(this->String);
    StUtfIterator<Type> anIterOther(theCompare.String);
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
    const size_t aSize = this->Size + theAppend.Size;
    Type* aString = stStrAlloc(aSize);
    stStrCopy((stUByte_t* )aString,              (const stUByte_t* )this->String,     this->Size);
    stStrCopy((stUByte_t* )aString + this->Size, (const stUByte_t* )theAppend.String, theAppend.Size);

    stStrFree(this->String);
    this->Size   = aSize;
    this->String = aString;
    this->Length += theAppend.Length;
    return (*this);
}

template<typename Type> inline
StStringUnicode<Type> StStringUnicode<Type>::subString(const size_t theStart,
                                                       const size_t theEnd) const {
    if(theStart >= theEnd) {
        return StStringUnicode<Type>();
    }
    for(StUtfIterator<Type> anIter(this->String); *anIter != 0; ++anIter) {
        if(anIter.getIndex() >= theStart) {
            return StStringUnicode<Type>(anIter.getBufferHere(), theEnd - theStart);
        }
    }
    return StStringUnicode<Type>();
}

template<typename Type> inline
const StStringUnicode<stUtf8_t> StStringUnicode<Type>::toUtf8() const {
    StStringUnicode<stUtf8_t> aCopy;
    aCopy.fromUnicode(this->String);
    return aCopy;
}

template<typename Type> inline
const StStringUnicode<stUtf16_t> StStringUnicode<Type>::toUtf16() const {
    StStringUnicode<stUtf16_t> aCopy;
    aCopy.fromUnicode(this->String);
    return aCopy;
}

template<typename Type> inline
const StStringUnicode<stUtf32_t> StStringUnicode<Type>::toUtf32() const {
    StStringUnicode<stUtf32_t> aCopy;
    aCopy.fromUnicode(this->String);
    return aCopy;
}

template<typename Type> inline
const StStringUnicode<stUtfWide_t> StStringUnicode<Type>::toUtfWide() const {
    StStringUnicode<stUtfWide_t> aCopy;
    aCopy.fromUnicode(this->String);
    return aCopy;
}

template<typename Type> inline
const StStringUnicode<stUtf8_t> StStringUnicode<Type>::toDump() const {
    StStringUnicode<stUtf8_t> aDump;
    aDump += StStringUnicode<stUtf8_t>("StStringUnicode<") + sizeof(Type) + ">, size= " + this->Size + ", length= " + this->Length + ", text= '";
    switch(sizeof(Type)) {
        case sizeof(stUtf8_t): {
            aDump += (const stUtf8_t* )this->String;
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
bool StConstStringUnicode<Type>::isStartsWith(const Type theStartChar) const {
    return this->Length != 0
        && this->String[0] == theStartChar;
}

template<typename Type> inline
bool StConstStringUnicode<Type>::isEndsWith(const Type theEndChar) const {
    return this->Length != 0
        && *(Type* )((stUByte_t* )this->String + this->Size - sizeof(Type)) == theEndChar;
}

template<typename Type> inline
bool StConstStringUnicode<Type>::isStartsWith(const StConstStringUnicode<Type>& theStartString) const {
    if(this->Size < theStartString.Size) {
        return false;
    } else if(this == &theStartString) {
        return true;
    }
    return stStrAreEqual((const char* )this->String,          theStartString.Size,
                         (const char* )theStartString.String, theStartString.Size);
}

template<typename Type> inline
bool StStringUnicode<Type>::isStartsWithIgnoreCase(const StConstStringUnicode<Type>& theStartString) const {
    return (this == &theStartString)
        || subString(0, theStartString.getLength()).isEqualsIgnoreCase(theStartString);
}

template<typename Type> inline
bool StConstStringUnicode<Type>::isEndsWith(const StConstStringUnicode<Type>& theEndString) const {
    if(this->Size < theEndString.Size) {
        return false;
    } else if(this == &theEndString) {
        return true;
    }

    return stStrAreEqual((const char* )this->String + this->Size - theEndString.Size, theEndString.Size,
                         (const char* )theEndString.String, theEndString.Size);
}

template<typename Type> inline
bool StStringUnicode<Type>::isEndsWithIgnoreCase(const StConstStringUnicode<Type>& theEndString) const {
    if(this == &theEndString) {
        return true;
    }
    return (this->Length >= theEndString.Length)
        && subString(this->Length - theEndString.Length, this->Length).isEqualsIgnoreCase(theEndString);
}

template<typename Type> inline
StHandle <StArrayList< StStringUnicode<Type> > > StStringUnicode<Type>::split(const stUtf32_t theDelimeter,
                                                                              const size_t    theLimitNb) const {
    StHandle< StArrayList< StStringUnicode<Type> > > aSplitList
        = new StArrayList< StStringUnicode<Type> >((theLimitNb < size_t(-1)) ? theLimitNb : 16);

    size_t aStart = 0;
    size_t aSplitCount = 1; // set to 1, that is
    for(StUtfIterator<Type> anIter(this->String);; ++anIter) {
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
                if(this->Length > aStart) {
                    aSplitList->add(subString(aStart, this->Length));
                }
                break;
            }
        }
    }
    return aSplitList;
}

template<typename Type> inline
bool StConstStringUnicode<Type>::isContains(const stUtf32_t theSubChar) const {
    for(StUtfIterator<Type> anIter(this->String); *anIter != 0; ++anIter) {
        if(stUtf32_t(*anIter) == theSubChar) {
            return true;
        }
    }
    return false;
}

template<typename Type> inline
bool StConstStringUnicode<Type>::isContains(const StConstStringUnicode<Type>& theSubString) const {
    if(theSubString.isEmpty()) {
        return true;
    }
    StUtfIterator<Type> anIterMe(this->String);
    StUtfIterator<Type> anIterSub(theSubString.String);
    for(;; ++anIterMe) {
        if(*anIterMe == 0) {
            return *anIterSub == 0;
        } else if(*anIterSub == 0) {
            return true;
        } else if(*anIterMe == *anIterSub) {
            ++anIterSub;
        } else {
            anIterSub.init(theSubString.String);
        }
    }
}

template<typename Type> inline
StStringUnicode<Type> StStringUnicode<Type>::unquoted() const {
    if(this->Length < 2) {
        return *this;
    }
    const Type* aLastSymbol = getCharBuffer(this->Length - 1);
    if((aLastSymbol != this->String)
    && ((*this->String == Type('\"') && *aLastSymbol == Type('\"'))
     || (*this->String == Type('\'') && *aLastSymbol == Type('\'')))) {
        return subString(1, this->Length - 1);
    }
    return *this;
}

template<typename Type> inline
StStringUnicode<Type> StStringUnicode<Type>::replace(const StStringUnicode<Type>& theSubString,
                                                     const StStringUnicode<Type>& theReplacer) const {
    if(theSubString.isEmpty() || this->isEmpty() || theSubString.Size > this->Size) {
        // just make a copy
        return *this;
    } else if(theSubString.Size == this->Size) {
        if(theSubString.isEquals(*this)) {
            return theReplacer;
        }
    }
    StUtfIterator<Type> anIter(this->String);
    StStringUnicode<Type> aResult;
    size_t aStart = 0;
    for(size_t aCharId = 0; *anIter != 0;) {
        if(stAreEqual(anIter.getBufferHere(), theSubString.String, theSubString.Size)) {
            aResult += subString(aStart, aCharId);
            aResult += theReplacer;
            aStart  = aCharId + theSubString.Length;
            aCharId = aStart;
            anIter.init(anIter.getBufferHere() + theSubString.Size);
            continue;
        }
        ++anIter;
        ++aCharId;
    }
    if(aStart < this->Length) {
        aResult += subString(aStart, this->Length);
    }
    return aResult;
}

template<typename Type> inline
void StStringUnicode<Type>::replaceFast(const StStringUnicode<Type>& theSubString,
                                        const StStringUnicode<Type>& theReplacer) {
    if(theSubString.isEmpty() || this->isEmpty() || theSubString.Size >= this->Size) {
        // just make a copy
        return;
    } else if(theSubString.Size != theReplacer.Size) {
        // invalid method usage
        ST_DEBUG_ASSERT(theSubString.Size == theReplacer.Size);
        ///*this = replace(theSubString, theReplacer);
        return;
    }
    for(StUtfIterator<Type> anIter(this->String); *anIter != 0;) {
        if(stAreEqual(anIter.getBufferHere(), theSubString.String, theSubString.Size)) {
            stStrCopy((stUByte_t* )anIter.changeBufferHere(), (const stUByte_t* )theReplacer.String, theReplacer.Size);
            anIter.init(anIter.getBufferHere() + theSubString.Size);
            continue;
        }
        ++anIter;
    }
}

template<typename Type> inline
void StStringUnicode<Type>::leftAdjust() {
    if(this->Length == 0) {
        return;
    }
    for(size_t anIter = 0; anIter < this->Size / sizeof(Type); ++anIter) {
      if(this->String[anIter] != (Type )' '
      && this->String[anIter] != (Type )'\t') {
          if(anIter != 0) {
              StStringUnicode<Type> aCopy(&this->String[anIter]);
              *this = aCopy;
          }
          return;
      }
    }
}

template<typename Type> inline
void StStringUnicode<Type>::rightAdjust() {
    if(this->Length == 0) {
        return;
    }
    for(size_t anIter = this->Size / sizeof(Type) - 1; anIter != 0; --anIter) {
      if(this->String[anIter] == (Type )' '
      || this->String[anIter] == (Type )'\t') {
          ((Type* )this->String)[anIter] = (Type )'\0';
          this->Size -= sizeof(Type);
          --this->Length;
      } else {
          return;
      }
    }
}

template<typename Type> inline
void StStringUnicode<Type>::toLowerCase() {
    for(StUtfIterator<Type> anIter(this->String); *anIter != 0; ++anIter) {
        if(*anIter > 64 && *anIter < 91) {
            *anIter.changeBufferHere() = Type(*anIter + 32);
        }
    }
}

template<typename Type> inline
StStringUnicode<Type> StStringUnicode<Type>::lowerCased() const {
    StStringUnicode<Type> aCopy(*this);
    aCopy.toLowerCase();
    return aCopy;
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

#ifdef _WIN32
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

#endif // __StStringUnicode_inl__
