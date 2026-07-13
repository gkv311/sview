/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StStrings/StDictionary.h>

const StString StDictEntry::ST_ARG_ON   ("on");
const StString StDictEntry::ST_ARG_TRUE ("true");
const StString StDictEntry::ST_ARG_OFF  ("off");
const StString StDictEntry::ST_ARG_FALSE("false");

namespace {
    static const StDictEntry THE_NULL_ENTRY;
}

StDictEntry::StDictEntry() {}

StDictEntry::StDictEntry(const StString& theKey)
: myKey(theKey) {
    //
}

StDictEntry::StDictEntry(const StString& theKey,
                         const StString& theValue)
: myKey(theKey),
  myValue(theValue.unquoted()) {
    //
}

StDictEntry::~StDictEntry() {
    //
}

StString StDictEntry::toString() const {
    return myKey + "=\"" + myValue + '\"';
}

bool StDictEntry::isValueOn() const {
    return myValue.isEqualsIgnoreCase(ST_ARG_ON)
        || myValue.isEqualsIgnoreCase(ST_ARG_TRUE);
}

bool StDictEntry::isValueOff() const {
    return myValue.isEqualsIgnoreCase(ST_ARG_OFF)
        || myValue.isEqualsIgnoreCase(ST_ARG_FALSE);
}

void StDictEntry::parseString(const StString& theString) {
    for(StUtf8Iter anIter = theString.iterator(); *anIter != 0; ++anIter) {
        if(*anIter == stUtf32_t('=')) {
            myKey   = theString.subString(0, anIter.getIndex());
            myValue = theString.subString(anIter.getIndex() + 1, theString.getLength()).unquoted();
            return;
        }
    }
    myKey = theString;
}

StDictionary::StDictionary()
: StArrayList<StDictEntry>() {
    //
}

StDictionary::~StDictionary() {
    //
}

StString StDictionary::toString() const {
    return StArrayList<StDictEntry>::toString();
}

void StDictionary::parseList(const StArrayList<StString>& theStringList) {
    for(size_t id = 0; id < theStringList.size(); ++id) {
        StDictEntry newArgument;
        newArgument.parseString(theStringList[id]);
        add(newArgument);
    }
}

void StDictionary::parseString(const StString& theString) {
    size_t aStart = 0;
    bool isInQuotes1 = false;
    bool isInQuotes2 = false;
    for(StUtf8Iter anIter = theString.iterator();; ++anIter) {
        if(*anIter == stUtf32_t('\'') && !isInQuotes2) {
            isInQuotes1 = !isInQuotes1;
        } else if(*anIter == stUtf32_t('\"') && !isInQuotes1) {
            isInQuotes2 = !isInQuotes2;
        } else if((*anIter == stUtf32_t('\n')
                || *anIter == stUtf32_t('\0')) && !isInQuotes1 && !isInQuotes2) {
            StDictEntry aNewArgument;
            aNewArgument.parseString(theString.subString(aStart, anIter.getIndex()));
            add(aNewArgument);
            aStart = anIter.getIndex() + 1;
        }
        if(*anIter == 0) {
            return;
        }
    }
}

const StDictEntry& StDictionary::operator[](const StString& theKey) const {
    for(size_t anId = 0; anId < size(); ++anId) {
        const StDictEntry& anArg = getValue(anId);
        if(anArg.getKey().isEqualsIgnoreCase(theKey)) {
            return anArg;
        }
    }
    return THE_NULL_ENTRY;
}

StDictEntry& StDictionary::addChange(const StString& theKey) {
    for(size_t anId = 0; anId < size(); ++anId) {
        StDictEntry& anArg = changeValue(anId);
        if(anArg.getKey().isEqualsIgnoreCase(theKey)) {
            return anArg;
        }
    }
    add(StDictEntry(theKey));
    return changeLast();
}

void StDictionary::set(const StDictEntry& thePair) {
    for(size_t anId = 0; anId < size(); ++anId) {
        StDictEntry& anArg = changeValue(anId);
        if(anArg.getKey().isEqualsIgnoreCase(thePair.getKey())) {
            anArg.setValue(thePair.getValue());
            return;
        }
    }
    add(thePair);
}
