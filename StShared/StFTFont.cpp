/**
 * Copyright © 2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StFT/StFTFont.h>

#include <StFile/StFileNode.h>
#include <StStrings/StLogger.h>

StFTFont::StFTFont(StHandle<StFTLibrary> theFTLib)
: myFTLib(theFTLib),
  myFTFace(NULL) {
    if(myFTLib.isNull()) {
        myFTLib = new StFTLibrary();
    }
}

StFTFont::~StFTFont() {
    release();
}

void StFTFont::release() {
    myGlyphImg.nullify();
    myFontPath.clear();
    myUChar = 0;
    if(myFTFace != NULL) {
        FT_Done_Face(myFTFace);
        myFTFace = NULL;
    }
}

bool StFTFont::init(const StString&    theFontPath,
                    const unsigned int thePointSize,
                    const unsigned int theResolution) {
    release();
    myFontPath = theFontPath;
    if(!myFTLib->isValid()) {
        ST_DEBUG_LOG("StFTFont, FreeType library is unavailable!");
        return false;
    }

    const StString aFontPath = StFileNode::getCompatibleName(myFontPath);
    if(FT_New_Face(myFTLib->getInstance(), aFontPath.toCString(), 0, &myFTFace) != 0) {
        ST_DEBUG_LOG("Font '" + aFontPath + "' fail to load!");
        release();
        return false;
    } else if(FT_Select_Charmap(myFTFace, ft_encoding_unicode) != 0) {
        ST_DEBUG_LOG("Font '" + aFontPath + "' doesn't contains Unicode charmap!");
        release();
        return false;
    } else if(FT_Set_Char_Size(myFTFace, 0L, thePointSize * 64, theResolution, theResolution) != 0) {
        ST_DEBUG_LOG("Font '" + aFontPath + "' doesn't contains Unicode charmap!");
        release();
        return false;
    }
    return true;
}

bool StFTFont::loadGlyph(const stUtf32_t theUChar) {
    if(myUChar == theUChar) {
        return true;
    }

    myGlyphImg.nullify();
    myUChar = 0;
    if(theUChar == 0
    || FT_Load_Char(myFTFace, theUChar, FT_LOAD_TARGET_NORMAL) != 0
    || myFTFace->glyph == NULL) {
        return false;
    }

    myUChar = theUChar;
    return true;
}

bool StFTFont::renderGlyph(const stUtf32_t theUChar) {
    myGlyphImg.nullify();
    myUChar = 0;
    // | FT_LOAD_NO_BITMAP
    if(theUChar == 0
    || FT_Load_Char(myFTFace, theUChar, FT_LOAD_RENDER | FT_LOAD_NO_HINTING | FT_LOAD_TARGET_NORMAL) != 0
    || myFTFace->glyph == NULL
    || myFTFace->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
        return false;
    }

    FT_Bitmap aBitmap = myFTFace->glyph->bitmap;
    if(aBitmap.pixel_mode != FT_PIXEL_MODE_GRAY
    || aBitmap.buffer == NULL || aBitmap.width <= 0 || aBitmap.rows <= 0) {
        return false;
    }
    if(!myGlyphImg.initWrapper(StImagePlane::ImgGray, aBitmap.buffer,
                               aBitmap.width, aBitmap.rows, std::abs(aBitmap.pitch))) {
        return false;
    }
    myGlyphImg.setTopDown(aBitmap.pitch > 0);
    myUChar = theUChar;
    return true;
}

unsigned int StFTFont::getGlyphMaxSizeX() const {
    float aWidth = (FT_IS_SCALABLE(myFTFace) != 0)
                 ? float(myFTFace->bbox.xMax - myFTFace->bbox.xMin) * (float(myFTFace->size->metrics.x_ppem) / float(myFTFace->units_per_EM))
                 : float(myFTFace->size->metrics.max_advance) / 64.0f;
    return (unsigned int)(aWidth + 0.5f);
}

unsigned int StFTFont::getGlyphMaxSizeY() const {
    float aHeight = (FT_IS_SCALABLE(myFTFace) != 0)
                  ? float(myFTFace->bbox.yMax - myFTFace->bbox.yMin) * (float(myFTFace->size->metrics.y_ppem) / float(myFTFace->units_per_EM))
                  : float(myFTFace->size->metrics.height) / 64.0f;
    return (unsigned int)(aHeight + 0.5f);
}

float StFTFont::getAdvanceX(const stUtf32_t theUChar,
                            const stUtf32_t theUCharNext) {
    loadGlyph(theUChar);
    return getAdvanceX(theUCharNext);
}

float StFTFont::getAdvanceY(const stUtf32_t theUChar,
                            const stUtf32_t theUCharNext) {
    loadGlyph(theUChar);
    return getAdvanceY(theUCharNext);
}

void StFTFont::addAdvanceX(const stUtf32_t  theUChar,
                           const stUtf32_t  theUCharNext,
                           StVec2<GLfloat>& thePen) {
    loadGlyph(theUChar);
    addAdvanceX(theUCharNext, thePen);
}

void StFTFont::addAdvanceY(const stUtf32_t  theUChar,
                           const stUtf32_t  theUCharNext,
                           StVec2<GLfloat>& thePen) {
    loadGlyph(theUChar);
    addAdvanceY(theUCharNext, thePen);
}

float StFTFont::getAdvanceX(const stUtf32_t theUCharNext) {
    if(myUChar == 0) {
        return 0.0f;
    }

    if(FT_HAS_KERNING(myFTFace) == 0 || theUCharNext == 0
    || FT_Get_Kerning(myFTFace, myUChar, theUCharNext, FT_KERNING_UNFITTED, &myKernAdvance) != 0) {
        return float(myFTFace->glyph->advance.x) / 64.0f;
    }
    return float(myKernAdvance.x + myFTFace->glyph->advance.x) / 64.0f;
}

float StFTFont::getAdvanceY(const stUtf32_t theUCharNext) {
    if(myUChar == 0) {
        return 0.0f;
    }

    if(FT_HAS_KERNING(myFTFace) == 0 || theUCharNext == 0
    || FT_Get_Kerning(myFTFace, myUChar, theUCharNext, FT_KERNING_UNFITTED, &myKernAdvance) != 0) {
        return float(myFTFace->glyph->advance.y) / 64.0f;
    }
    return float(myKernAdvance.y + myFTFace->glyph->advance.y) / 64.0f;
}

void StFTFont::addAdvanceX(const stUtf32_t  theUCharNext,
                           StVec2<GLfloat>& thePen) {
    if(myUChar == 0) {
        return;
    }

    if(FT_HAS_KERNING(myFTFace) == 0 || theUCharNext == 0
    || FT_Get_Kerning(myFTFace, myUChar, theUCharNext, FT_KERNING_UNFITTED, &myKernAdvance) != 0) {
        thePen.x() += float(myFTFace->glyph->advance.x) / 64.0f;
    } else {
        thePen.x() += float(myKernAdvance.x + myFTFace->glyph->advance.x) / 64.0f;
    }
}

void StFTFont::addAdvanceY(const stUtf32_t  theUCharNext,
                           StVec2<GLfloat>& thePen) {
    if(myUChar == 0) {
        return;
    }

    if(FT_HAS_KERNING(myFTFace) == 0 || theUCharNext == 0
    || FT_Get_Kerning(myFTFace, myUChar, theUCharNext, FT_KERNING_UNFITTED, &myKernAdvance) != 0) {
        thePen.y() += float(myFTFace->glyph->advance.y) / 64.0f;
    } else {
        thePen.y() += float(myKernAdvance.y + myFTFace->glyph->advance.y) / 64.0f;
    }
}
