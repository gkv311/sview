/**
 * Copyright © 2013-2025 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGL/StGLFontManager.h>

#include <StStrings/StLogger.h>
#include <stAssert.h>

namespace {

    // fallback font
    #include "StDejaVuSerif.ttf.h"

}

StGLFontManager::StGLFontManager(const unsigned int theResolution)
: myFTLib(std::make_shared<StFTLibrary>()),
  myResolution(theResolution) {
    myRegistry = std::make_shared<StFTFontRegistry>();
    myRegistry->init(false);
}

StGLFontManager::~StGLFontManager() {
    //
}

void StGLFontManager::release(StGLContext& theCtx) {
    for (std::map< StGLFontKey, std::shared_ptr<StGLFontEntry> >::iterator anIter = myFonts.begin();
        anIter != myFonts.end(); ++anIter) {
        if (anIter->second.get() != nullptr) {
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

std::shared_ptr<StGLFontEntry> StGLFontManager::find(const StString& theName,
                                              unsigned int    theSize) const {
    std::map< StGLFontKey, std::shared_ptr<StGLFontEntry> >::const_iterator aFontIter = myFonts.find(StGLFontKey(theName, theSize));
    return (aFontIter != myFonts.end()) ? aFontIter->second : std::shared_ptr<StGLFontEntry>();
}

std::shared_ptr<StGLFontEntry> StGLFontManager::findCreate(const StString& theName,
                                                           unsigned int    theSize) {
    std::shared_ptr<StGLFontEntry>& aFontGl = myFonts[StGLFontKey(theName, theSize)];
    if (aFontGl.get() != nullptr) {
        return aFontGl;
    }

    const StFTFontFamily& aFont = myRegistry->findFont(theName);
    if (aFont.FamilyName.isEmpty()) {
        return std::shared_ptr<StGLFontEntry>();
    }

    std::shared_ptr<StFTFont> aFontFt = std::make_shared<StFTFont>(myFTLib);
    const bool hasItalic     = !aFont.Italic.isEmpty();
    const bool hasBoldItalic = !aFont.BoldItalic.isEmpty();
    aFontFt->load(aFont.Regular, aFont.RegularFace,    StFTFont::Style_Regular);
    aFontFt->load(aFont.Bold,    aFont.BoldFace,       StFTFont::Style_Bold);
    aFontFt->load(hasItalic ? aFont.Italic : aFont.Regular,
                  hasItalic ? aFont.ItalicFace : aFont.RegularFace,
                  StFTFont::Style_Italic, !hasItalic);
    aFontFt->load(hasBoldItalic ? aFont.BoldItalic : aFont.Bold,
                  hasBoldItalic ? aFont.BoldItalicFace : aFont.BoldFace,
                  StFTFont::Style_BoldItalic, !hasBoldItalic);
    aFontFt->init(theSize, myResolution);

    aFontGl = std::make_shared<StGLFontEntry>(aFontFt);
    return aFontGl;
}

std::shared_ptr<StGLFontEntry> StGLFontManager::findCreateFallback(unsigned int theSize) {
    const StString aName = getFallbackFontName();
    std::shared_ptr<StGLFontEntry>& aFontGl = myFonts[StGLFontKey(aName, theSize)];
    if (aFontGl.get() != nullptr) {
        return aFontGl;
    }

    ST_ERROR_LOG("StGLFontManager, fallback font is used!");
    std::shared_ptr<StFTFont> aFontFt = std::make_shared<StFTFont>(myFTLib);
    aFontFt->loadInternal(getFallbackFontPath(),
                          THE_DejaVuSerif_ttf_DATA,
                          THE_DejaVuSerif_ttf_LEN,
                          StFTFont::Style_Regular);
    aFontFt->init(theSize, myResolution);

    aFontGl = std::make_shared<StGLFontEntry>(aFontFt);
    return aFontGl;
}

const std::shared_ptr<StGLFont>& StGLFontManager::findCreate(const StFTFont::Typeface theType,
                                                             unsigned int             theSize) {
    std::shared_ptr<StGLFont>& aFont = myFontTypes[StGLFontTypeKey(theType, theSize)];
    if (aFont.get() != nullptr) {
        return aFont;
    }

    aFont = std::make_shared<StGLFont>();
    const StFTFontPack& aPack = myRegistry->getTypeface(theType);

    std::shared_ptr<StGLFontEntry>& aGenFont = aFont->changeFont(StFTFont::Subset_General);
    aGenFont                                    = findCreate(aPack.Western.FamilyName, theSize);
    aFont->changeFont(StFTFont::Subset_CJK)     = findCreate(aPack.CJK    .FamilyName, theSize);
    aFont->changeFont(StFTFont::Subset_Korean)  = findCreate(aPack.Korean .FamilyName, theSize);
    if (aGenFont.get() == nullptr || !aGenFont->hasSubset(StFTFont::Subset_Arabic)) {
        aFont->changeFont(StFTFont::Subset_Arabic) = findCreate(aPack.Arabic.FamilyName, theSize);
    }
    if (aGenFont.get() == nullptr || !aGenFont->hasSubset(StFTFont::Subset_MiscSymbols)) {
        aFont->changeFont(StFTFont::Subset_MiscSymbols) = findCreate(aPack.MiscSymbols.FamilyName, theSize);
    }

    if (aGenFont.get() == nullptr) {
        aGenFont = findCreateFallback(theSize);
    }
    return aFont;
}
