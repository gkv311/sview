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
     * Default constructor from FT font with regular style.
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
     * @return textured font instance of specified style
     */
    ST_LOCAL const StHandle<StGLFontEntry>& getFont(const StFTFont::Style theStyle) const {
        return myFonts[theStyle];
    }

    /**
     * @return textured font instance of specified style
     */
    ST_LOCAL StHandle<StGLFontEntry>& changeFont(const StFTFont::Style theStyle) {
        return myFonts[theStyle];
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
        return !myFonts[StFTFont::Style_Regular].isNull()
             && myFonts[StFTFont::Style_Regular]->wasInitialized();
    }

    /**
     * @return vertical distance from the horizontal baseline to the highest character coordinate.
     */
    ST_LOCAL inline GLfloat getAscender() const {
        return myAscender;
    }

    /**
     * @return default line spacing (the baseline-to-baseline distance).
     */
    ST_LOCAL inline GLfloat getLineSpacing() const {
        return myLineSpacing;
    }

        protected:

    StHandle<StGLFontEntry> myFonts   [StFTFont::StylesNB]; //!< textured font instances
    StHandle<StGLFontEntry> myFontsCJK[StFTFont::StylesNB]; //!< textured font instances (CJK back-end)

    StString myFamily;      //!< font family name
    GLfloat  myAscender;    //!< ascender     provided my FT font
    GLfloat  myLineSpacing; //!< line spacing provided my FT font

};

#endif // __StGLFont_h_
