/**
 * Copyright © 2013-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGL/StGLFont.h>

StGLFont::StGLFont() {
    //
}

StGLFont::StGLFont(const StHandle<StFTFont>& theFtFont) {
    myFonts[0] = new StGLFontEntry(theFtFont);
}

StGLFont::~StGLFont() {
    //
}

void StGLFont::release(StGLContext& theCtx) {
    for(size_t anIter = 0; anIter < StFTFont::SubsetsNB; ++anIter) {
        StHandle<StGLFontEntry>& aFont = myFonts[anIter];
        if(!aFont.isNull()) {
            aFont->release(theCtx);
        }
    }
}

bool StGLFont::stglInit(StGLContext&       theCtx,
                        const unsigned int thePointSize,
                        const unsigned int theResolution) {
    StHandle<StGLFontEntry>& aFontMain = myFonts[0];
    if(aFontMain.isNull()
    || !aFontMain->stglInit(theCtx, thePointSize, theResolution)) {
        return false;
    }

    for(size_t anIter = 1; anIter < StFTFont::SubsetsNB; ++anIter) {
        StHandle<StGLFontEntry>& aFont = myFonts[anIter];
        if(!aFont.isNull()) {
            aFont->stglInit(theCtx, thePointSize, theResolution, false);
        }
    }
    return true;
}

bool StGLFont::stglInit(StGLContext& theCtx) {
    StHandle<StGLFontEntry>& aFontMain = myFonts[0];
    if(aFontMain.isNull()
    || !aFontMain->stglInit(theCtx)) {
        return false;
    }

    for(size_t anIter = 1; anIter < StFTFont::SubsetsNB; ++anIter) {
        StHandle<StGLFontEntry>& aFont = myFonts[anIter];
        if(!aFont.isNull()) {
            aFont->stglInit(theCtx, false);
        }
    }
    return true;
}

bool StGLFont::setActiveStyle(const StFTFont::Style theStyle) {
    bool hasStyle = false;
    for(size_t anIter = 0; anIter < StFTFont::SubsetsNB; ++anIter) {
        StHandle<StGLFontEntry>& aFont = myFonts[anIter];
        if(!aFont.isNull()) {
            hasStyle = aFont->setActiveStyle(theStyle) || hasStyle;
        }
    }
    return hasStyle;
}

void StGLFont::renderGlyph(StGLContext&    theCtx,
                           const stUtf32_t theUChar,
                           const stUtf32_t theUCharNext,
                           StGLTile&       theGlyph,
                           StGLVec2&       thePen) {
    const StFTFont::Subset aSubset = StFTFont::subset(theUChar);
    StHandle<StGLFontEntry>& aFont = myFonts[aSubset];
    if(!aFont.isNull()
    && aFont->hasSymbol(theUChar)
    && aFont->renderGlyph(theCtx, false, theUChar, theUCharNext, theGlyph, thePen)) {
        return;
    }
    myFonts[0]->renderGlyph(theCtx, true, theUChar, theUCharNext, theGlyph, thePen);
}
