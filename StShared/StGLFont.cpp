/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGL/StGLFont.h>

StGLFont::StGLFont()
: myFamily("UNDEFINED"),
  myAscender(0.0f),
  myLineSpacing(0.0f) {
    //
}

StGLFont::StGLFont(const StHandle<StFTFont>& theFtFont)
: myFamily(theFtFont->getFamilyName()),
  myAscender(0.0f),
  myLineSpacing(0.0f) {
    myFonts[StFTFont::Style_Regular] = new StGLFontEntry(theFtFont);
}

StGLFont::~StGLFont() {
    //
}

void StGLFont::release(StGLContext& theCtx) {
    for(size_t anIter = 0; anIter < StFTFont::StylesNB; ++anIter) {
        StHandle<StGLFontEntry>& aFont    = myFonts   [anIter];
        StHandle<StGLFontEntry>& aFontCJK = myFontsCJK[anIter];
        if(!aFont.isNull()) {
            aFont->release(theCtx);
        }
        if(!aFontCJK.isNull()) {
            aFontCJK->release(theCtx);
        }
    }
}

bool StGLFont::stglInit(StGLContext&       theCtx,
                        const unsigned int thePointSize,
                        const unsigned int theResolution) {
    bool isOk = true;
    myAscender    = 0.0f;
    myLineSpacing = 0.0f;
    for(size_t anIter = 0; anIter < StFTFont::StylesNB; ++anIter) {
        StHandle<StGLFontEntry>& aFont    = myFonts   [anIter];
        StHandle<StGLFontEntry>& aFontCJK = myFontsCJK[anIter];
        if(!aFont.isNull()) {
            isOk = aFont->stglInit(theCtx, thePointSize, theResolution) && isOk;
            myAscender    = stMax(myAscender,    aFont->getAscender());
            myLineSpacing = stMax(myLineSpacing, aFont->getLineSpacing());
        }
        if(!aFontCJK.isNull()) {
            aFontCJK->stglInit(theCtx, thePointSize, theResolution);
        }
    }
    return isOk;
}

bool StGLFont::stglInit(StGLContext& theCtx) {
    bool isOk = true;
    myAscender    = 0.0f;
    myLineSpacing = 0.0f;
    for(size_t anIter = 0; anIter < StFTFont::StylesNB; ++anIter) {
        StHandle<StGLFontEntry>& aFont    = myFonts   [anIter];
        StHandle<StGLFontEntry>& aFontCJK = myFontsCJK[anIter];
        if(!aFont.isNull()) {
            isOk = aFont->stglInit(theCtx) && isOk;
            myAscender    = stMax(myAscender,    aFont->getAscender());
            myLineSpacing = stMax(myLineSpacing, aFont->getLineSpacing());
        }
        if(!aFontCJK.isNull()) {
            aFontCJK->stglInit(theCtx);
        }
    }
    return isOk;
}
