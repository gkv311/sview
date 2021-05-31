/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StFreeImage_h_
#define __StFreeImage_h_

#include "StImageFile.h"

// define StHandle template specialization
class StFreeImage;
ST_DEFINE_HANDLE(StFreeImage, StImageFile);

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

    ST_LOCAL virtual StHandle<StImageFile> createEmpty() const ST_ATTR_OVERRIDE { return new StFreeImage(); }

    ST_CPPEXPORT virtual void close() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool loadExtra(const StString& theFilePath,
                                        ImageType       theImageType,
                                        uint8_t*        theDataPtr,
                                        int             theDataSize,
                                        bool            theIsOnlyRGB) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool save(const StString& theFilePath,
                                   ImageType       theImageType,
                                   StFormat        theSrcFormat) ST_ATTR_OVERRIDE;

        private:

    typedef struct FIBITMAP FIBITMAP;
    FIBITMAP* myDIB;

};

#endif //__StFreeImage_h_
