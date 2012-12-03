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
class ST_LOCAL StFreeImage : public StImageFile {

        private:

    typedef struct FIBITMAP FIBITMAP;
    FIBITMAP* myDIB;

        public:

    /**
     * Should be called at application start.
     */
    static bool init();

        public:

    StFreeImage();
    virtual ~StFreeImage();

    virtual void close();
    virtual bool load(const StString& theFilePath,
                      ImageType theImageType = ST_TYPE_NONE,
                      uint8_t* theDataPtr = NULL, int theDataSize = 0);
    virtual bool save(const StString& theFilePath,
                      ImageType theImageType);
    virtual bool resize(size_t , size_t ) { return false; }
};

#endif //__StFreeImage_h_
