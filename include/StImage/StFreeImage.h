/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StFreeImage_h_
#define __StFreeImage_h_

#include "StImageFile.h"

/**
 * This class implements image load/save operations using FreeImage library.
 */
class StFreeImage : public StImageFile {

        public:

    /**
     * Should be called at application start.
     */
    ST_CPPEXPORT static bool init();

        public:

    ST_CPPEXPORT StFreeImage();
    ST_CPPEXPORT virtual ~StFreeImage();

    ST_CPPEXPORT virtual void close();
    ST_CPPEXPORT virtual bool load(const StString& theFilePath,
                                   ImageType theImageType = ST_TYPE_NONE,
                                   uint8_t* theDataPtr = NULL, int theDataSize = 0);
    ST_CPPEXPORT virtual bool save(const StString& theFilePath,
                                   ImageType       theImageType,
                                   StFormatEnum    theSrcFormat);
    ST_CPPEXPORT virtual bool resize(size_t , size_t );

        private:

    typedef struct FIBITMAP FIBITMAP;
    FIBITMAP* myDIB;

};

#endif //__StFreeImage_h_
