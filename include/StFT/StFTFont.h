/**
 * Copyright © 2012-2013 Kirill Gavrilov <kirill@sview.ru>
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
 * Class to read font using FreeType library.
 * Notice that this class uses internal buffers for loaded glyphs
 * and it is absolutely UNSAFE to load/read glyph from concurrent threads!
 */
class StFTFont {

        public:

    enum Style {
      Style_Regular,
      Style_Bold,
      Style_Italic,
      Style_BoldItalic,
      StylesNB
    };

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
     * Initialize the font.
     * @param theFontPath   path to the font
     * @param thePointSize  the face size in points (1/72 inch)
     * @param theResolution the resolution of the target device
     * @return true on success
     */
    ST_CPPEXPORT bool init(const StString&    theFontPath,
                           const unsigned int thePointSize,
                           const unsigned int theResolution = 72);

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
     */
    ST_CPPEXPORT bool renderGlyph(const stUtf32_t theChar);

    /**
     * @return maximal glyph width in pixels (rendered to bitmap).
     */
    ST_CPPEXPORT unsigned int getGlyphMaxSizeX() const;

    /**
     * @return maximal glyph height in pixels (rendered to bitmap).
     */
    ST_CPPEXPORT unsigned int getGlyphMaxSizeY() const;

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

        protected:

    /**
     * Load glyph without rendering it.
     */
    ST_CPPEXPORT bool loadGlyph(const stUtf32_t theUChar);

        protected:

    StHandle<StFTLibrary> myFTLib;       //!< handle to the FT library object
    FT_Face               myFTFace;      //!< FT face object
    StString              myFontPath;    //!< font path

    StImagePlane          myGlyphImg;    //!< cached glyph plane
    FT_Vector             myKernAdvance; //!< buffer variable
    stUtf32_t             myUChar;       //!< currently loaded unicode character

};

#endif // __StFTFont_h_
