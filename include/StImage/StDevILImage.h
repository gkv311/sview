/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StDevILImage_h_
#define __StDevILImage_h_

#include "StImageFile.h"

typedef unsigned int ILuint;

/**
 * This class implements image load/save operations using DevIL library.
 */
class StDevILImage : public StImageFile {

        private:

    ILuint myImageId;

        private:

    inline bool isValid() const {
        return myImageId != 0;
    }

        public:

    /**
     * Should be called at application start.
     */
    ST_CPPEXPORT static bool init();

        public:

    ST_CPPEXPORT StDevILImage();
    ST_CPPEXPORT virtual ~StDevILImage();

    ST_CPPEXPORT virtual void close();
    ST_CPPEXPORT virtual bool load(const StString& theFilePath,
                                   ImageType theImageType = ST_TYPE_NONE,
                                   uint8_t* theDataPtr = NULL, int theDataSize = 0);
    ST_CPPEXPORT virtual bool save(const StString& theFilePath,
                                   ImageType theImageType);
    ST_CPPEXPORT virtual bool resize(size_t theSizeX, size_t theSizeY);

};

#endif //__StDevILImage_h_
