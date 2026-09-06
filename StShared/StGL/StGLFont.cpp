/**
 * Copyright © 2013-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGL/StGLFont.h>

StGLFont::StGLFont() {
    //
}

StGLFont::StGLFont(const std::shared_ptr<StFTFont>& theFtFont) {
    myFonts[0] = std::make_shared<StGLFontEntry>(theFtFont);
}

StGLFont::~StGLFont() {
    //
}

void StGLFont::release(StGLContext& theCtx) {
    for (size_t anIter = 0; anIter < StFTFont::SubsetsNB; ++anIter) {
        std::shared_ptr<StGLFontEntry>& aFont = myFonts[anIter];
        if (aFont.get() != nullptr) {
            aFont->release(theCtx);
        }
    }
}

bool StGLFont::stglInit(StGLContext&       theCtx,
                        const unsigned int thePointSize,
                        const unsigned int theResolution) {
    std::shared_ptr<StGLFontEntry>& aFontMain = myFonts[0];
    if (aFontMain.get() == nullptr
    || !aFontMain->stglInit(theCtx, thePointSize, theResolution)) {
        return false;
    }

    for (size_t anIter = 1; anIter < StFTFont::SubsetsNB; ++anIter) {
        std::shared_ptr<StGLFontEntry>& aFont = myFonts[anIter];
        if (aFont.get() != nullptr) {
            aFont->stglInit(theCtx, thePointSize, theResolution, false);
        }
    }
    return true;
}

bool StGLFont::stglInit(StGLContext& theCtx) {
    std::shared_ptr<StGLFontEntry>& aFontMain = myFonts[0];
    if (aFontMain.get() == nullptr
    || !aFontMain->stglInit(theCtx)) {
        return false;
    }

    for (size_t anIter = 1; anIter < StFTFont::SubsetsNB; ++anIter) {
        std::shared_ptr<StGLFontEntry>& aFont = myFonts[anIter];
        if (aFont.get() != nullptr) {
            aFont->stglInit(theCtx, false);
        }
    }
    return true;
}

bool StGLFont::setActiveStyle(const StFTFont::Style theStyle) {
    bool hasStyle = false;
    for (size_t anIter = 0; anIter < StFTFont::SubsetsNB; ++anIter) {
        std::shared_ptr<StGLFontEntry>& aFont = myFonts[anIter];
        if (aFont.get() != nullptr) {
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
    std::shared_ptr<StGLFontEntry>& aFont = myFonts[aSubset];
    if (aFont.get() != nullptr
     && aFont->hasSymbol(theUChar)
     && aFont->renderGlyph(theCtx, false, theUChar, theUCharNext, theGlyph, thePen)) {
        return;
    }
    myFonts[0]->renderGlyph(theCtx, true, theUChar, theUCharNext, theGlyph, thePen);
}
