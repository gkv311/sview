/**
 * Copyright © 2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StStbImage_h_
#define __StStbImage_h_

#include "StImageFile.h"

/**
 * This class implements image load/save operations using STB library.
 */
class StStbImage : public StImageFile {

        public:

    /**
     * Should be called at application start.
     */
    ST_CPPEXPORT static bool init();

        public:

    ST_CPPEXPORT StStbImage();
    ST_CPPEXPORT virtual ~StStbImage();

    ST_LOCAL virtual std::shared_ptr<StImageFile> createEmpty() const ST_ATTR_OVERRIDE { return std::make_shared<StStbImage>(); }

    ST_CPPEXPORT virtual void close() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool loadExtra(const StString& theFilePath,
                                        ImageType       theImageType,
                                        uint8_t*        theDataPtr,
                                        int             theDataSize,
                                        bool            theIsOnlyRGB) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool save(const StString& theFilePath,
                                   const SaveImageParams& theParams) ST_ATTR_OVERRIDE;


        private:

     void* myStbImage = nullptr;

};

#endif //__StStbImage_h_
