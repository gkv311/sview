/**
 * Copyright © 2012-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLFontEntry_h_
#define __StGLFontEntry_h_

#include <StFT/StFTFont.h>
#include <StGL/StGLTexture.h>
#include <StGL/StGLFrameBuffer.h>
#include <StGL/StGLVec.h>
#include <StTemplates/StRect.h>

#include <map>

typedef StRect<GLfloat> StGLRect;

/**
 * Simple structure stores tile rectangle.
 */
struct StGLTile {

    StGLRect uv;      //!< UV coordinates in texture
    StGLRect px;      //!< pixel displacement coordinates
    GLuint   texture; //!< GL texture ID

        public: //! compare operators

    bool operator==(const StGLTile& theCompare) const {
        return uv == theCompare.uv && px == theCompare.px;
    }

};

template<> inline void StArray< StHandle<StGLTexture> >::sort() {}
template<> inline void StArray< StHandle<StGLFrameBuffer> >::sort() {}
template<> inline void StArray<StGLTile>::sort() {}
template<> inline void StArray<StGLRect>::sort() {}

/**
 * Texture font.
 */
class StGLFontEntry : public StGLResource {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLFontEntry(const StHandle<StFTFont>& theFont);

    /**
     * Destructor - should be called after release()!
     */
    ST_CPPEXPORT virtual ~StGLFontEntry();

    /**
     * Release GL resources.
     */
    ST_CPPEXPORT virtual void release(StGLContext& theCtx) ST_ATTR_OVERRIDE;

    /**
     * @return FreeType font instance specified on construction.
     */
    inline StHandle<StFTFont>& getFont() {
        return myFont;
    }

    /**
     * @return active font style
     */
    ST_LOCAL StFTFont::Style getActiveStyle() const {
        return myFont->getActiveStyle();
    }

    /**
     * @return setup active font style
     */
    ST_CPPEXPORT bool setActiveStyle(const StFTFont::Style theStyle);

    /**
     * @return true if font was loaded successfully.
     */
    inline bool isValid() const {
        return !myTextures.isEmpty() && myTextures[0]->isValid();
    }

    /**
     * Notice that this method doesn't return initialization success state.
     * Use isValid() instead.
     * @return true if initialization was already called.
     */
    inline bool wasInitialized() const {
        return !myTextures.isEmpty();
    }

    /**
     * Initialize GL resources.
     * FreeType font instance should be already initialized!
     */
    ST_CPPEXPORT bool stglInit(StGLContext& theCtx,
                               const bool   theToCreateTexture = true);

    /**
     * Re-initialize GL resources.
     */
    ST_CPPEXPORT bool stglInit(StGLContext&       theCtx,
                               const unsigned int thePointSize,
                               const unsigned int theResolution,
                               const bool         theToCreateTexture = true);

    /**
     * Compute advance to the next character with kerning applied when applicable.
     * Assuming text rendered horizontally.
     */
    ST_LOCAL inline float getAdvanceX(const stUtf32_t theUChar,
                                      const stUtf32_t theUCharNext) {
        return myFont->getAdvanceX(theUChar, theUCharNext);
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

    /**
     * @return true if font contains specified symbol
     */
    ST_LOCAL bool hasSymbol(const stUtf32_t theUChar) const {
        return !myFont.isNull()
             && myFont->hasSymbol(theUChar);
    }

    /**
     * @return true if this font contains glyphs of specified subset
     */
    ST_LOCAL bool hasSubset(StFTFont::Subset theSubset) const {
        return !myFont.isNull()
             && myFont->hasSubset(theSubset);
    }

    /**
     * Compute glyph rectangle at specified pen position (on baseline)
     * and render it to texture if not already.
     * @param theCtx         active context
     * @param theToDrawUndef when true than undefined character is drawn as empty rectangle
     * @param theUChar       unicode symbol to render
     * @param theUCharNext   next symbol to compute advance with kerning when available
     * @param theGlyph       computed glyph position rectangle, texture ID and UV coordinates
     * @param thePen         pen position on baseline to place new glyph
     * @return true if font contains specified character
     */
    ST_CPPEXPORT bool renderGlyph(StGLContext&    theCtx,
                                  const bool      theToDrawUndef,
                                  const stUtf32_t theUChar,
                                  const stUtf32_t theUCharNext,
                                  StGLTile&       theGlyph,
                                  StGLVec2&       thePen);

        protected:

    /**
     * Render new glyph to the texture.
     */
    ST_CPPEXPORT bool renderGlyph(StGLContext&    theCtx,
                                  const stUtf32_t theChar,
                                  const bool      theToForce);

    /**
     * Allocate new texture.
     */
    ST_CPPEXPORT bool createTexture(StGLContext& theCtx);

        protected:

    StHandle<StFTFont> myFont;                //!< FreeType font instance
    GLfloat            myAscender;            //!< ascender     provided my FT font
    GLfloat            myLineSpacing;         //!< line spacing provided my FT font
    GLsizei            myTileSizeX;           //!< tile width
    GLsizei            myTileSizeY;           //!< tile height
    size_t             myLastTileId;          //!< id of last tile
    StRect<int>        myLastTilePx;

    StArrayList< StHandle<StGLTexture> >     myTextures; //!< texture list
    StArrayList< StHandle<StGLFrameBuffer> > myFbos;     //!< FBO list
    StArrayList<StGLTile> myTiles;            //!< tiles list

    std::map<stUtf32_t, size_t>  myGlyphMaps[StFTFont::StylesNB];
    std::map<stUtf32_t, size_t>* myGlyphMap;                      //!< glyphs map for active style

};

#endif // __StGLFontEntry_h_
