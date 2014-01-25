/**
 * Copyright © 2012-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StFTFont_h_
#define __StFTFont_h_

#include <StStrings/StString.h>
#include <StFT/StFTLibrary.h>
#include <StImage/StImagePlane.h>
#include <StTemplates/StVec2.h>
#include <StTemplates/StRect.h>

/**
 * Structure holds the paths to font family files.
 */
struct StFTFontFamily {
    StString FamilyName;

    StString Regular;
    StString Bold;
    StString Italic;
    StString BoldItalic;

    ST_LOCAL StFTFontFamily() {}
};

struct StFTFontPack {
    StFTFontFamily Western;
    StFTFontFamily CJK;
    StFTFontFamily Korean;
};

/**
 * Class to read font using FreeType library.
 * Notice that this class uses internal buffers for loaded glyphs
 * and it is absolutely UNSAFE to load/read glyph from concurrent threads!
 */
class StFTFont {

        public:

    /**
     * Font styles.
     */
    enum Style {
        Style_Regular,      //!< normal letters
        Style_Bold,         //!< wide letters
        Style_Italic,       //!< oblique letters
        Style_BoldItalic,   //!< wide oblique letters
        StylesNB
    };

    /**
     * Typefaces.
     */
    enum Typeface {
        Typeface_Serif,     //!< contains small features at the end of strokes within letters
        Typeface_SansSerif, //!< without serifs
        Typeface_Monospace, //!< non-proportional / fixed-width
        TypefacesNB
    };

    /**
     * Unicode subsets.
     */
    enum Subset {
        Subset_General,
        Subset_Korean,  //!< modern Korean letters
        Subset_CJK,     //!< Chinese characters (Chinese, Japanese, Korean and Vietnam)
        SubsetsNB
    };

    /**
     * @return true if specified character is within subset of modern CJK characters
     */
    ST_LOCAL static bool isCJK(const stUtf32_t theUChar) {
        return (theUChar >= 0x03400 && theUChar <= 0x04DFF)
            || (theUChar >= 0x04E00 && theUChar <= 0x09FFF)
            || (theUChar >= 0x0F900 && theUChar <= 0x0FAFF)
            || (theUChar >= 0x20000 && theUChar <= 0x2A6DF)
            || (theUChar >= 0x2F800 && theUChar <= 0x2FA1F);
    }

    /**
     * @return true if specified character is within subset of modern Korean characters (Hangul)
     */
    ST_LOCAL static bool isKorean(const stUtf32_t theUChar) {
        return (theUChar >= 0x01100 && theUChar <= 0x011FF)
            || (theUChar >= 0x03130 && theUChar <= 0x0318F)
            || (theUChar >= 0x0AC00 && theUChar <= 0x0D7A3);
    }

    /**
     * Determine Unicode subset for specified character
     */
    ST_LOCAL static Subset subset(const stUtf32_t theUChar) {
        if(isCJK(theUChar)) {
            return Subset_CJK;
        } else if(isKorean(theUChar)) {
            return Subset_Korean;
        }
        return Subset_General;
    }

        public:

    /**
     * Create uninitialized instance.
     */
    ST_CPPEXPORT StFTFont(StHandle<StFTLibrary> theFTLib = NULL);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StFTFont();

    /**
     * @return true if font is loaded.
     */
    inline bool isValid() const {
        return myFTFace != NULL;
    }

    /**
     * @return image plane for currently rendered glyph.
     */
    inline const StImagePlane& getGlyphImage() const {
        return myGlyphImg;
    }

    /**
     * Compute glyph rectangle.
     */
    inline void getGlyphRect(StRect<float>& theRect) const {
        FT_Bitmap aBitmap = myFTFace->glyph->bitmap;
        theRect.left()   = float(myFTFace->glyph->bitmap_left);
        theRect.top()    = float(myFTFace->glyph->bitmap_top);
        theRect.right()  = float(myFTFace->glyph->bitmap_left + aBitmap.width);
        theRect.bottom() = float(myFTFace->glyph->bitmap_top  - aBitmap.rows);
    }

    /**
     * Load the font from specified path.
     * @param theFontPath path to the font
     * @param theStyle    the font style
     * @return true on success
     */
    ST_CPPEXPORT bool load(const StString&       theFontPath,
                           const StFTFont::Style theStyle);

    /**
     * Re-initialize the font.
     * @param thePointSize  the face size in points (1/72 inch)
     * @param theResolution the resolution of the target device
     * @return true on success
     */
    ST_CPPEXPORT bool init(const unsigned int thePointSize,
                           const unsigned int theResolution = 72);

    /**
     * Release currently loaded font.
     */
    ST_CPPEXPORT void release();

    /**
     * Render specified glyph into internal buffer (bitmap).
     * Notice that this method ignores notdef symbol!
     */
    ST_CPPEXPORT bool renderGlyph(const stUtf32_t theChar);

    /**
     * Render special notdef glyph into internal buffer (bitmap).
     */
    ST_CPPEXPORT bool renderGlyphNotdef();

    /**
     * @return true if font contains specified symbol
     */
    ST_CPPEXPORT bool hasSymbol(const stUtf32_t theUChar) const;

    /**
     * @return maximal glyph width in pixels (rendered to bitmap).
     */
    ST_LOCAL unsigned int getGlyphMaxSizeX() const {
        return myGlyphMaxWidth;
    }

    /**
     * @return maximal glyph height in pixels (rendered to bitmap).
     */
    ST_LOCAL unsigned int getGlyphMaxSizeY() const {
        return myGlyphMaxHeight;
    }

    /**
     * @return vertical distance from the horizontal baseline to the highest character coordinate.
     */
    inline float getAscender() const {
        return float(myFTFace->ascender) * (float(myFTFace->size->metrics.y_ppem) / float(myFTFace->units_per_EM));
    }

    /**
     * @return default line spacing (the baseline-to-baseline distance).
     */
    inline float getLineSpacing() const {
        return float(myFTFace->height) * (float(myFTFace->size->metrics.y_ppem) / float(myFTFace->units_per_EM));
    }

    /**
     * Compute advance to the next character with kerning applied when applicable
     * and append it to pen position.
     * Assuming text rendered horizontally.
     */
    ST_CPPEXPORT void addAdvanceX(const stUtf32_t  theUCharNext,
                                  StVec2<GLfloat>& thePen);

    /**
     * Compute advance to the next character with kerning applied when applicable
     * and append it to pen position.
     * Assuming text rendered horizontally.
     */
    ST_CPPEXPORT void addAdvanceX(const stUtf32_t  theUChar,
                                  const stUtf32_t  theUCharNext,
                                  StVec2<GLfloat>& thePen);

    /**
     * Compute advance to the next character with kerning applied when applicable
     * and append it to pen position.
     * Assuming text rendered vertically.
     */
    ST_CPPEXPORT void addAdvanceY(const stUtf32_t  theUCharNext,
                                  StVec2<GLfloat>& thePen);

    /**
     * Compute advance to the next character with kerning applied when applicable
     * and append it to pen position.
     * Assuming text rendered vertically.
     */
    ST_CPPEXPORT void addAdvanceY(const stUtf32_t  theUChar,
                                  const stUtf32_t  theUCharNext,
                                  StVec2<GLfloat>& thePen);

    /**
     * Compute advance to the next character with kerning applied when applicable.
     * Assuming text rendered horizontally.
     */
    ST_CPPEXPORT float getAdvanceX(const stUtf32_t theUCharNext);

    /**
     * Compute advance to the next character with kerning applied when applicable.
     * Assuming text rendered horizontally.
     */
    ST_CPPEXPORT float getAdvanceX(const stUtf32_t theUChar,
                                   const stUtf32_t theUCharNext);

    /**
     * Compute advance to the next character with kerning applied when applicable.
     * Assuming text rendered vertically.
     */
    ST_CPPEXPORT float getAdvanceY(const stUtf32_t theUCharNext);

    /**
     * Compute advance to the next character with kerning applied when applicable.
     * Assuming text rendered vertically.
     */
    ST_CPPEXPORT float getAdvanceY(const stUtf32_t theUChar,
                                   const stUtf32_t theUCharNext);

    /**
     * @return glyphs number in this font.
     */
    ST_LOCAL inline int getGlyphsNumber() const {
        return myFTFace->num_glyphs;
    }

    /**
     * @return family name as reported by font file itself
     */
    ST_LOCAL StString getFamilyName() const {
        return myFTFace->family_name;
    }

    /**
     * @return font path
     */
    ST_LOCAL const StString& getFilePath(const StFTFont::Style theStyle) const {
        return myFontPaths[theStyle];
    }

    /**
     * @return true if this font contains CJK (Chinese, Japanese, and Korean) glyphs
     */
    ST_LOCAL bool hasCJK() const {
        return mySubsets[Subset_CJK];
    }

    /**
     * @return true if this font contains Korean glyphs
     */
    ST_LOCAL bool hasKorean() const {
        return mySubsets[Subset_Korean];
    }

        protected:

    /**
     * Load glyph without rendering it.
     */
    ST_CPPEXPORT bool loadGlyph(const stUtf32_t theUChar);

        protected:

    StHandle<StFTLibrary> myFTLib;               //!< handle to the FT library object
    FT_Face               myFTFace;              //!< active FT face object
    FT_Face               myFTFaces[StylesNB];   //!< FT face objects
    StString              myFontPaths[StylesNB]; //!< font paths
    bool                  mySubsets[SubsetsNB];
    FT_Int32              myLoadFlags;           //!< default load flags
    unsigned int          myGlyphMaxWidth;       //!< maximum glyph width
    unsigned int          myGlyphMaxHeight;      //!< maximum glyph height

    StImagePlane          myGlyphImg;            //!< cached glyph plane
    FT_Vector             myKernAdvance;         //!< buffer variable
    stUtf32_t             myUChar;               //!< currently loaded unicode character

};

#endif // __StFTFont_h_
