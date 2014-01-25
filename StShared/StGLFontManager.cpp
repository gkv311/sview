/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGL/StGLFontManager.h>

#include <StStrings/StLogger.h>
#include <stAssert.h>

StGLFontManager::StGLFontManager(const unsigned int theResolution)
: myFTLib(new StFTLibrary()),
  myResolution(theResolution) {
    myRegistry = new StFTFontRegistry();
    myRegistry->init(false);
}

StGLFontManager::~StGLFontManager() {
    //
}

void StGLFontManager::release(StGLContext& theCtx) {
    for(std::map< StGLFontKey, StHandle<StGLFontEntry> >::iterator anIter = myFonts.begin();
        anIter != myFonts.end(); ++anIter) {
        if(!anIter->second.isNull()) {
            anIter->second->release(theCtx);
        }
    }
    myFonts.clear();
    myFontTypes.clear();
}

void StGLFontManager::setResolution(const unsigned int theResolution) {
    if(myResolution == theResolution) {
        return;
    }

    myResolution = theResolution;
}

StHandle<StGLFontEntry> StGLFontManager::find(const StString& theName,
                                              unsigned int    theSize) const {
    std::map< StGLFontKey, StHandle<StGLFontEntry> >::const_iterator aFontIter = myFonts.find(StGLFontKey(theName, theSize));
    return (aFontIter != myFonts.end()) ? aFontIter->second : StHandle<StGLFontEntry>();
}

StHandle<StGLFontEntry> StGLFontManager::findCreate(const StString& theName,
                                                    unsigned int    theSize) {
    StHandle<StGLFontEntry>& aFontGl = myFonts[StGLFontKey(theName, theSize)];
    if(!aFontGl.isNull()) {
        return aFontGl;
    }

    const StFTFontFamily& aFont = myRegistry->findFont(theName);
    if(aFont.FamilyName.isEmpty()) {
        return StHandle<StGLFontEntry>();
    }

    StHandle<StFTFont> aFontFt = new StFTFont(myFTLib);
    aFontFt->load(aFont.Regular,    StFTFont::Style_Regular);
    aFontFt->load(aFont.Bold,       StFTFont::Style_Bold);
    aFontFt->load(aFont.Italic,     StFTFont::Style_Italic);
    aFontFt->load(aFont.BoldItalic, StFTFont::Style_BoldItalic);
    aFontFt->init(theSize, myResolution);
    aFontGl = new StGLFontEntry(aFontFt);
    return aFontGl;
}

const StHandle<StGLFont>& StGLFontManager::findCreate(const StFTFont::Typeface theType,
                                                      unsigned int             theSize) {
    StHandle<StGLFont>& aFont = myFontTypes[StGLFontTypeKey(theType, theSize)];
    if(!aFont.isNull()) {
        return aFont;
    }

    aFont = new StGLFont();
    const StFTFontPack& aPack = myRegistry->getTypeface(theType);
    aFont->changeFont(StFTFont::Subset_General) = findCreate(aPack.Western.FamilyName, theSize);
    aFont->changeFont(StFTFont::Subset_CJK)     = findCreate(aPack.CJK    .FamilyName, theSize);
    aFont->changeFont(StFTFont::Subset_Korean)  = findCreate(aPack.Korean .FamilyName, theSize);
    return aFont;
}
