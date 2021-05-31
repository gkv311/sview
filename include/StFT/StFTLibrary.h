/**
 * Copyright © 2012-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StFTLibrary_h_
#define __StFTLibrary_h_

#include <stTypes.h>

// idiotic inclusion template for FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

/**
 * Class wrapper over FT_Library.
 * Provides access to FreeType library.
 */
class StFTLibrary {

        public:

    /**
     * Initialize new FT_Library instance.
     */
    ST_CPPEXPORT StFTLibrary();

    /**
     * Release FT_Library instance.
     */
    ST_CPPEXPORT ~StFTLibrary();

    /**
     * This method should always return true.
     * @return true if FT_Library instance is valid.
     */
    inline bool isValid() const {
        return myFTLib != NULL;
    }

    /**
     * Access FT_Library instance.
     */
    inline FT_Library getInstance() const {
        return myFTLib;
    }

        private:

    FT_Library myFTLib;

        private:

    StFTLibrary(const StFTLibrary& );
    StFTLibrary& operator=(const StFTLibrary& );

};

#endif // __StFTLibrary_h_
