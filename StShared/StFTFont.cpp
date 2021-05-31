/**
 * Copyright © 2012-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StFT/StFTFont.h>

#include <StFile/StFileNode.h>
#include <StStrings/StLogger.h>

StFTFont::StFTFont(StHandle<StFTLibrary> theFTLib)
: myFTLib(theFTLib),
  myFTFace(NULL),
  myStyle(StFTFont::Style_Regular),
  myLoadFlags(FT_LOAD_NO_HINTING | FT_LOAD_TARGET_NORMAL),
  myGlyphMaxWidth(1),
  myGlyphMaxHeight(1),
  myUChar(0) {
    if(myFTLib.isNull()) {
        myFTLib = new StFTLibrary();
    }
    stMemZero(mySubsets, sizeof(mySubsets));
    stMemZero(myFTFaces, sizeof(myFTFaces));
    stMemZero(myFontFaces, sizeof(myFontFaces));
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
        myFontFaces[aStyleIt] = 0;
    }
}

bool StFTFont::setActiveStyle(const StFTFont::Style theStyle) {
    if(myStyle == theStyle) {
        return true;
    }

    if(myFTFaces[theStyle] != NULL) {
        myUChar  = 0;
        myStyle  = theStyle;
        myFTFace = myFTFaces[theStyle];
        return true;
    }
    if(!hasCJK()
    && !hasKorean()) {
        // simulate style using transformation
    }
    return false;
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
    myFTFace = myFTFaces[myStyle];
    return true;
}

bool StFTFont::load(const StString& theFontPath,
                    const int theFaceId,
                    const StFTFont::Style theStyle,
                    const bool theToSyntItalic) {
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
    myFontFaces[theStyle] = theFaceId;

    FT_Face& aFace = myFTFaces[theStyle];
    if(aFace != NULL) {
        FT_Done_Face(aFace);
    }
    const StString aFontPath = StFileNode::getCompatibleName(theFontPath);
    if(FT_New_Face(myFTLib->getInstance(), aFontPath.toCString(), (FT_Long )theFaceId, &aFace) != 0) {
        ST_DEBUG_LOG("Font '" + aFontPath + "' fail to load!");
        FT_Done_Face(aFace);
        aFace = NULL;
        return false;
    }

    if(theToSyntItalic) {
        const double THE_SHEAR_ANGLE = 10.0 * M_PI / 180.0;

        FT_Matrix aMat;
        aMat.xx = FT_Fixed(std::cos(-THE_SHEAR_ANGLE) * (1 << 16));
        aMat.xy = 0;
        aMat.yx = 0;
        aMat.yy = aMat.xx;

        FT_Fixed aFactor = FT_Fixed(std::tan(THE_SHEAR_ANGLE) * (1 << 16));
        aMat.xy += FT_MulFix(aFactor, aMat.xx);

        FT_Set_Transform(aFace, &aMat, 0);
    }
    return loadCharmap(aFontPath, aFace);
}

bool StFTFont::loadInternal(const StString&       theFontName,
                            const unsigned char*  theFontData,
                            const int             theDataLen,
                            const StFTFont::Style theStyle) {
    if(!myFTLib->isValid()
    || theStyle <  Style_Regular
    || theStyle >= StylesNB
    || theFontData == NULL
    || theDataLen  < 1) {
        return false;
    }
    myUChar  = 0;
    myFTFace = NULL;
    myGlyphImg.nullify();
    myFontPaths[theStyle] = theFontName;
    myFontFaces[theStyle] = 0;

    FT_Face& aFace = myFTFaces[theStyle];
    if(aFace != NULL) {
        FT_Done_Face(aFace);
    }

    if(FT_New_Memory_Face(myFTLib->getInstance(), theFontData, theDataLen, 0, &aFace) != 0) {
        ST_DEBUG_LOG("Font '" + theFontName + "' fail to load!");
        FT_Done_Face(aFace);
        aFace = NULL;
        return false;
    }
    return loadCharmap(theFontName, aFace);
}

bool StFTFont::loadCharmap(const StString& theFontName,
                           FT_Face&        theFace) {
    (void )theFontName;
    if(FT_Select_Charmap(theFace, ft_encoding_unicode) != 0) {
        ST_DEBUG_LOG("Font '" + theFontName + "' doesn't contain Unicode charmap!");
        FT_Done_Face(theFace);
        theFace = NULL;
        return false;
    }

    // test Unicode subsets
    mySubsets[Subset_General] = true;
    mySubsets[Subset_Korean]  =     FT_Get_Char_Index(theFace, 0x0B371) != 0
                                 && FT_Get_Char_Index(theFace, 0x0D130) != 0;
    mySubsets[Subset_CJK]         = FT_Get_Char_Index(theFace, 0x06F22) != 0;
    mySubsets[Subset_Arabic]      = FT_Get_Char_Index(theFace, 0x00600) != 0;
    mySubsets[Subset_MiscSymbols] = FT_Get_Char_Index(theFace, 0x0266A) != 0;
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

inline bool wrapGlyphBitmap(const FT_Bitmap& theBitmap,
                            StImagePlane&    theOutImage) {
    const unsigned int aWidth  = (unsigned int )theBitmap.width;
    const unsigned int aHeight = (unsigned int )theBitmap.rows;
    if(theBitmap.buffer == NULL
    || aWidth  == 0
    || aHeight == 0) {
        return false;
    }

    if(theBitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
        if(!theOutImage.initWrapper(StImagePlane::ImgGray, theBitmap.buffer,
                                    aWidth, aHeight, std::abs(theBitmap.pitch))) {
            return false;
        }
    } else if(theBitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
        if(!theOutImage.initTrash(StImagePlane::ImgGray, aWidth, aHeight)) {
            return false;
        }

        const int aNumOfBytesInRow = aWidth / 8 + (aWidth % 8 ? 1 : 0);
        for(unsigned int aRow = 0; aRow < aHeight; ++aRow) {
            for(unsigned int aCol = 0; aCol < aWidth; ++aCol) {
                const int aBitOn = theBitmap.buffer[aNumOfBytesInRow * aRow + aCol / 8] & (0x80 >> (aCol % 8));
                theOutImage.changeFirstByte(aRow, aCol) = aBitOn ? 255 : 0;
            }
        }
    } else {
        return false;
    }
    theOutImage.setTopDown(theBitmap.pitch > 0);
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

    if(!wrapGlyphBitmap(myFTFace->glyph->bitmap, myGlyphImg)) {
        return false;
    }

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

    return wrapGlyphBitmap(myFTFace->glyph->bitmap, myGlyphImg);
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

bool StFTFont::getKerning(FT_Vector&      theKern,
                          const stUtf32_t theUCharCurr,
                          const stUtf32_t theUCharNext) const {
    theKern.x = 0;
    theKern.y = 0;
    if(theUCharNext != 0 && FT_HAS_KERNING(myFTFace) != 0) {
        const FT_UInt aCharCurr = FT_Get_Char_Index(myFTFace, theUCharCurr);
        const FT_UInt aCharNext = FT_Get_Char_Index(myFTFace, theUCharNext);
        if(aCharCurr == 0 || aCharNext == 0
        || FT_Get_Kerning (myFTFace, aCharCurr, aCharNext, FT_KERNING_UNFITTED, &theKern) != 0) {
            theKern.x = 0;
            theKern.y = 0;
            return false;
        }
        return true;
    }
    return false;
}

float StFTFont::getAdvanceX(const stUtf32_t theUCharNext) const {
    if(myUChar == 0) {
        return 0.0f;
    }

    FT_Vector aKern;
    getKerning(aKern, myUChar, theUCharNext);
    return float(aKern.x + myFTFace->glyph->advance.x) / 64.0f;
}

float StFTFont::getAdvanceY(const stUtf32_t theUCharNext) const {
    if(myUChar == 0) {
        return 0.0f;
    }

    FT_Vector aKern;
    getKerning(aKern, myUChar, theUCharNext);
    return float(aKern.y + myFTFace->glyph->advance.y) / 64.0f;
}

void StFTFont::addAdvanceX(const stUtf32_t  theUCharNext,
                           StVec2<GLfloat>& thePen) const {
    if(myUChar == 0) {
        return;
    }

    FT_Vector aKern;
    getKerning(aKern, myUChar, theUCharNext);
    thePen.x() += float(aKern.x + myFTFace->glyph->advance.x) / 64.0f;
}

void StFTFont::addAdvanceY(const stUtf32_t  theUCharNext,
                           StVec2<GLfloat>& thePen) const {
    if(myUChar == 0) {
        return;
    }

    FT_Vector aKern;
    getKerning(aKern, myUChar, theUCharNext);
    thePen.y() += float(aKern.y + myFTFace->glyph->advance.y) / 64.0f;
}

bool StFTFont::hasSymbol(const stUtf32_t theUChar) const {
    return FT_Get_Char_Index(myFTFace, theUChar) != 0;
}
