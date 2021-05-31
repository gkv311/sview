/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StDevILImage_h_
#define __StDevILImage_h_

#include "StImageFile.h"

typedef unsigned int ILuint;

// define StHandle template specialization
class StDevILImage;
ST_DEFINE_HANDLE(StDevILImage, StImageFile);

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

    ST_CPPEXPORT bool resize(size_t theSizeX, size_t theSizeY);

        public:

    ST_CPPEXPORT StDevILImage();
    ST_CPPEXPORT virtual ~StDevILImage();

    ST_LOCAL virtual StHandle<StImageFile> createEmpty() const ST_ATTR_OVERRIDE { return new StDevILImage(); }

    ST_CPPEXPORT virtual void close() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool loadExtra(const StString& theFilePath,
                                        ImageType       theImageType,
                                        uint8_t*        theDataPtr,
                                        int             theDataSize,
                                        bool            theIsOnlyRGB) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool save(const StString& theFilePath,
                                   ImageType       theImageType,
                                   StFormat        theSrcFormat) ST_ATTR_OVERRIDE;

};

#endif //__StDevILImage_h_
