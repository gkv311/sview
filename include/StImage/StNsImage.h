/**
 * Copyright © 2026 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StNsImage_h_
#define __StNsImage_h_

#include "StImageFile.h"

// define StHandle template specialization
class StNsImage;
ST_DEFINE_HANDLE(StNsImage, StImageFile);

#ifdef __OBJC__
@class NSBitmapImageRep;
#else
struct NSBitmapImageRep;
#endif

/**
 * This class implements image load/save operations using NSImage from AppKit library (macOS).
 */
class StNsImage : public StImageFile {

        public:

    /**
     * Should be called at application start.
     */
    ST_CPPEXPORT static bool init();

        public:

    ST_CPPEXPORT StNsImage();
    ST_CPPEXPORT virtual ~StNsImage();

    ST_LOCAL virtual StHandle<StImageFile> createEmpty() const ST_ATTR_OVERRIDE { return new StNsImage(); }

    ST_CPPEXPORT virtual void close() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool loadExtra(const StString& theFilePath,
                                        ImageType       theImageType,
                                        uint8_t*        theDataPtr,
                                        int             theDataSize,
                                        bool            theIsOnlyRGB) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool save(const StString& theFilePath,
                                   const SaveImageParams& theParams) ST_ATTR_OVERRIDE;

        private:

    NSBitmapImageRep* myNSBitmap = nullptr;

};

#endif //__StNsImage_h_
