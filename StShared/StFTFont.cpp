/**
 * Copyright © 2012-2014 Kirill Gavrilov <kirill@sview.ru>
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
  myFTFace(NULL),
  myLoadFlags(FT_LOAD_NO_HINTING | FT_LOAD_TARGET_NORMAL),
  myGlyphMaxWidth(1),
  myGlyphMaxHeight(1),
  myUChar(0) {
    if(myFTLib.isNull()) {
        myFTLib = new StFTLibrary();
    }
    stMemZero(mySubsets, sizeof(mySubsets));
    stMemZero(myFTFaces, sizeof(myFTFaces));
}

StFTFont::~StFTFont() {
    release();
}

void StFTFont::release() {
    myUChar  = 0;
    myFTFace = NULL;
    myGlyphImg.nullify();
    myGlyphMaxWidth  = 1;
    myGlyphMaxHeight = 1;
    for(size_t aStyleIt = 0; aStyleIt < StylesNB; ++aStyleIt) {
        FT_Face& aFace = myFTFaces[aStyleIt];
        if(aFace != NULL) {
            FT_Done_Face(aFace);
            aFace = NULL;
        }
        myFontPaths[aStyleIt].clear();
    }
}

bool StFTFont::init(const unsigned int thePointSize,
                    const unsigned int theResolution) {
    myUChar  = 0;
    myFTFace = NULL;
    myGlyphImg.nullify();
    myGlyphMaxWidth  = 1;
    myGlyphMaxHeight = 1;
    if(myFTFaces[Style_Regular] == NULL) {
        return false;
    }

    for(size_t aStyleIt = 0; aStyleIt < StylesNB; ++aStyleIt) {
        FT_Face& aFace = myFTFaces[aStyleIt];
        if(aFace == NULL) {
            continue;
        } else if(FT_Set_Char_Size(aFace, 0L, thePointSize * 64, theResolution, theResolution) != 0) {
            ST_ERROR_LOG("Font '" + myFontPaths[aStyleIt] + "' doesn't contain requested size!");
            return false;
        }

        float aWidth = (FT_IS_SCALABLE(aFace) != 0)
                     ? float(aFace->bbox.xMax - aFace->bbox.xMin) * (float(aFace->size->metrics.x_ppem) / float(aFace->units_per_EM))
                     : float(aFace->size->metrics.max_advance) / 64.0f;
        float aHeight = (FT_IS_SCALABLE(aFace) != 0)
                      ? float(aFace->bbox.yMax - aFace->bbox.yMin) * (float(aFace->size->metrics.y_ppem) / float(aFace->units_per_EM))
                      : float(aFace->size->metrics.height) / 64.0f;
        myGlyphMaxWidth  = stMax(myGlyphMaxWidth,  (unsigned int)(aWidth  + 0.5f));
        myGlyphMaxHeight = stMax(myGlyphMaxHeight, (unsigned int)(aHeight + 0.5f));

        /*myFTFace = myFTFaces[aStyleIt];
        if(myFTFace != NULL) {
            ST_DEBUG_LOG("Font '" + myFTFace->family_name + "'[" + aStyleIt + "] maxSize= " + myGlyphMaxWidth + "x" + myGlyphMaxHeight
                       + " lineSize= " + getLineSpacing());
        }*/
    }
    myFTFace = myFTFaces[Style_Regular];
    return true;
}

bool StFTFont::load(const StString&       theFontPath,
                    const StFTFont::Style theStyle) {
    if(!myFTLib->isValid()
    || theStyle <  Style_Regular
    || theStyle >= StylesNB
    || theFontPath.isEmpty()) {
        return false;
    }
    myUChar  = 0;
    myFTFace = NULL;
    myGlyphImg.nullify();
    myFontPaths[theStyle] = theFontPath;

    FT_Face& aFace = myFTFaces[theStyle];
    if(aFace != NULL) {
        FT_Done_Face(aFace);
    }
    const StString aFontPath = StFileNode::getCompatibleName(theFontPath);
    if(FT_New_Face(myFTLib->getInstance(), aFontPath.toCString(), 0, &aFace) != 0) {
        ST_DEBUG_LOG("Font '" + aFontPath + "' fail to load!");
        FT_Done_Face(aFace);
        aFace = NULL;
        return false;
    } else if(FT_Select_Charmap(aFace, ft_encoding_unicode) != 0) {
        ST_DEBUG_LOG("Font '" + aFontPath + "' doesn't contain Unicode charmap!");
        FT_Done_Face(aFace);
        aFace = NULL;
        return false;
    }

    // test Unicode subsets
    mySubsets[Subset_General] = true;
    mySubsets[Subset_Korean]  = FT_Get_Char_Index(aFace, 0x0B371) != 0
                             && FT_Get_Char_Index(aFace, 0x0D130) != 0;
    mySubsets[Subset_CJK]     = FT_Get_Char_Index(aFace, 0x06F22) != 0;

//if(mySubsets[Subset_Korean]) { std::cerr << "  found Korean in " << myFontPath << "\n"; }
//if(mySubsets[Subset_CJK])    { std::cerr << "  found CJK    in " << myFontPath << "\n"; }

    return true;
}

bool StFTFont::loadGlyph(const stUtf32_t theUChar) {
    if(myUChar == theUChar) {
        return myUChar != 0;
    }

    myGlyphImg.nullify();
    myUChar = 0;
    if(theUChar == 0
    || FT_Load_Char(myFTFace, theUChar, myLoadFlags) != 0
    || myFTFace->glyph == NULL) {
        return false;
    }

    myUChar = theUChar;
    return true;
}

bool StFTFont::renderGlyph(const stUtf32_t theUChar) {
    myGlyphImg.nullify();
    myUChar = 0;

    const FT_UInt aGlyphIndex = theUChar != 0
                              ? FT_Get_Char_Index(myFTFace, theUChar)
                              : 0;
    // | FT_LOAD_NO_BITMAP
    if(aGlyphIndex == 0
    || FT_Load_Glyph(myFTFace, aGlyphIndex, myLoadFlags | FT_LOAD_RENDER) != 0
    || myFTFace->glyph == NULL
    || myFTFace->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
        return false;
    }

    FT_Bitmap aBitmap = myFTFace->glyph->bitmap;
    if(aBitmap.buffer == NULL || aBitmap.width <= 0 || aBitmap.rows <= 0) {
        return false;
    }

    if(aBitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
        if(!myGlyphImg.initWrapper(StImagePlane::ImgGray, aBitmap.buffer,
                                   aBitmap.width, aBitmap.rows, std::abs(aBitmap.pitch))) {
            return false;
        }
    } else if(aBitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
        if(!myGlyphImg.initTrash(StImagePlane::ImgGray, aBitmap.width, aBitmap.rows)) {
            return false;
        }

        const int aNumOfBytesInRow = aBitmap.width / 8 + (aBitmap.width % 8 ? 1 : 0);
        for(int aRow = 0; aRow < aBitmap.rows; ++aRow) {
            for(int aCol = 0; aCol < aBitmap.width; ++aCol) {
                const int aBitOn = aBitmap.buffer[aNumOfBytesInRow * aRow + aCol / 8] & (0x80 >> (aCol % 8));
                myGlyphImg.changeFirstByte(aRow, aCol) = aBitOn ? 255 : 0;
            }
        }
    } else {
        return false;
    }
    myGlyphImg.setTopDown(aBitmap.pitch > 0);
    myUChar = theUChar;
    return true;
}

bool StFTFont::renderGlyphNotdef() {
    myGlyphImg.nullify();
    myUChar = 0;

    if(FT_Load_Glyph(myFTFace, 0, myLoadFlags | FT_LOAD_RENDER) != 0
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
    return true;
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

bool StFTFont::hasSymbol(const stUtf32_t theUChar) const {
    return FT_Get_Char_Index(myFTFace, theUChar) != 0;
}
