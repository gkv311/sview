/**
 * Copyright © 2013-2025 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLFontManager_h_
#define __StGLFontManager_h_

#include <StGL/StGLFont.h>
#include <StFT/StFTFontRegistry.h>

#include <map>

struct StGLFontKey {
    StString     Name; //!< font family name
    unsigned int Size; //!< font size

    ST_LOCAL StGLFontKey(const StString& theName,
                         unsigned int    theSize) : Name(theName), Size(theSize) {}

    ST_LOCAL bool operator<(const StGLFontKey& theOther) const {
        return this->Size < theOther.Size
            || (this->Size == theOther.Size
             && this->Name <  theOther.Name);
    }
};

struct StGLFontTypeKey {
    StFTFont::Typeface Type; //!< font typeface
    unsigned int       Size; //!< font size

    ST_LOCAL StGLFontTypeKey(const StFTFont::Typeface theTypeface,
                            unsigned int             theSize) : Type(theTypeface), Size(theSize) {}

    ST_LOCAL bool operator<(const StGLFontTypeKey& theOther) const {
        return this->Size < theOther.Size
            || (this->Size == theOther.Size
             && this->Type <  theOther.Type);
    }
};

/**
 * Font manager.
 */
class StGLFontManager : public StGLResource {

        public:

    /** Return name of fallback font entry. */
    ST_LOCAL static constexpr const char* getFallbackFontName() {
        return "ST_DejaVuSerif_ttf";
    }

    /** Return path of fallback font. */
    ST_LOCAL static constexpr const char* getFallbackFontPath() {
        return "internal://DejaVuSerif_internal.ttf";
    }

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLFontManager(const unsigned int theResolution = 72);

    /**
     * Destructor - should be called after release()!
     */
    ST_CPPEXPORT virtual ~StGLFontManager();

    /**
     * Release GL resources.
     */
    ST_CPPEXPORT virtual void release(StGLContext& theCtx) ST_ATTR_OVERRIDE;

    /**
     * @return resolution
     */
    ST_LOCAL unsigned int getResolution() const {
        return myResolution;
    }

    /**
     * Setup resolution.
     */
    ST_CPPEXPORT void setResolution(const unsigned int theResolution);

    /**
     * Find font and create it if not already created.
     */
    ST_CPPEXPORT const std::shared_ptr<StGLFont>& findCreate(const StFTFont::Typeface theType,
                                                             unsigned int             theSize);

    /**
     * Find font for specified family name and size.
     */
    ST_CPPEXPORT std::shared_ptr<StGLFontEntry> find(const StString& theName,
                                                     unsigned int    theSize) const;

    /**
     * Find font for specified family name and size, and create it if not already created.
     */
    ST_CPPEXPORT std::shared_ptr<StGLFontEntry> findCreate(const StString& theName,
                                                           unsigned int    theSize);

    /**
     * Find fallback font for specified size, and create it if not already created.
     */
    ST_CPPEXPORT std::shared_ptr<StGLFontEntry> findCreateFallback(unsigned int theSize);

    /**
     * @return handle to the FT library object
     */
    ST_LOCAL const std::shared_ptr<StFTLibrary>& getLibrary() const {
        return myFTLib;
    }

        protected:

    std::shared_ptr<StFTLibrary>      myFTLib;    //!< handle to the FT library object
    std::shared_ptr<StFTFontRegistry> myRegistry; //!< fonts registry

    std::map< StGLFontKey,
              std::shared_ptr<StGLFontEntry> > myFonts; //!< fonts map
    std::map< StGLFontTypeKey,
              std::shared_ptr<StGLFont> > myFontTypes;  //!< font typefaces map

    unsigned int myResolution; //!< fonts resolution

};

#endif // __StGLFontManager_h_
