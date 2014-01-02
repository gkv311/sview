/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLFont_h_
#define __StGLFont_h_

#include <StGL/StGLFontEntry.h>

/**
 * Texture font family.
 */
class StGLFont : public StGLResource {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StGLFont();

    /**
     * Default constructor from FT font.
     */
    ST_CPPEXPORT StGLFont(const StHandle<StFTFont>& theFtFont);

    /**
     * Destructor - should be called after release()!
     */
    ST_CPPEXPORT virtual ~StGLFont();

    /**
     * Release GL resources.
     */
    ST_CPPEXPORT virtual void release(StGLContext& theCtx);

    /**
     * @return textured font instance of specified Unicode subset
     */
    ST_LOCAL const StHandle<StGLFontEntry>& getFont(const StFTFont::Subset theSubset = StFTFont::Subset_General) const {
        return myFonts[theSubset];
    }

    /**
     * @return textured font instance of specified Unicode subset
     */
    ST_LOCAL StHandle<StGLFontEntry>& changeFont(const StFTFont::Subset theSubset = StFTFont::Subset_General) {
        return myFonts[theSubset];
    }

    /**
     * Initialize GL resources.
     * FreeType font instance should be already initialized!
     */
    ST_CPPEXPORT bool stglInit(StGLContext& theCtx);

    /**
     * Re-initialize GL resources.
     */
    ST_CPPEXPORT bool stglInit(StGLContext&       theCtx,
                               const unsigned int thePointSize,
                               const unsigned int theResolution);

    /**
     * Notice that this method doesn't return initialization success state.
     * @return true if initialization was already called.
     */
    inline bool wasInitialized() const {
        return !myFonts[0].isNull()
             && myFonts[0]->wasInitialized();
    }

    /**
     * Compute glyph rectangle at specified pen position (on baseline)
     * and render it to texture if not already.
     * @param theCtx       active context
     * @param theStyle     font style
     * @param theUChar     unicode symbol to render
     * @param theUCharNext next symbol to compute advance with kerning when available
     * @param theGlyph     computed glyph position rectangle, texture ID and UV coordinates
     * @param thePen       pen position on baseline to place new glyph
     */
    ST_CPPEXPORT void renderGlyph(StGLContext&          theCtx,
                                  const StFTFont::Style theStyle,
                                  const stUtf32_t       theUChar,
                                  const stUtf32_t       theUCharNext,
                                  StGLTile&             theGlyph,
                                  StGLVec2&             thePen);

        protected:

    StHandle<StGLFontEntry> myFonts[StFTFont::SubsetsNB]; //!< textured font instances

};

#endif // __StGLFont_h_
