/**
 * Copyright © 2026 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StWicImage_h_
#define __StWicImage_h_

#include "StImageFile.h"

// define StHandle template specialization
class StWicImage;
ST_DEFINE_HANDLE(StWicImage, StImageFile);

/**
 * This class implements image load/save operations using WinCodec library (WinAPI).
 */
class StWicImage : public StImageFile {

        public:

    /**
     * Should be called at application start.
     */
    ST_CPPEXPORT static bool init();

        public:

    ST_CPPEXPORT StWicImage();
    ST_CPPEXPORT virtual ~StWicImage();

    ST_LOCAL virtual StHandle<StImageFile> createEmpty() const ST_ATTR_OVERRIDE { return new StWicImage(); }

    ST_CPPEXPORT virtual void close() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool loadExtra(const StString& theFilePath,
                                        ImageType       theImageType,
                                        uint8_t*        theDataPtr,
                                        int             theDataSize,
                                        bool            theIsOnlyRGB) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool save(const StString& theFilePath,
                                   const SaveImageParams& theParams) ST_ATTR_OVERRIDE;

};

#endif //__StWicImage_h_
