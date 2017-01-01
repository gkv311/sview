/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#ifndef __StImageOcct_h_
#define __StImageOcct_h_

#include <StImage/StImageFile.h>

#include <Image_PixMap.hxx>

/**
 * OCCT image wrapper over StImage.
 */
class StImageOcct : public Image_PixMap {

    DEFINE_STANDARD_RTTI_INLINE(StImageOcct, Image_PixMap)

        public:

    /**
     * Empty constructor.
     */
    ST_LOCAL StImageOcct();

    /**
     * Destructor.
     */
    ST_LOCAL virtual ~StImageOcct();

    /**
     * Clear the image.
     */
    ST_LOCAL virtual void Clear() Standard_OVERRIDE;

    /**
     * Initialize image plane with required dimensions.
     * theSizeRowBytes - will be ignored by this class and required alignment will be used instead!
     */
    virtual bool InitTrash(ImgFormat           thePixelFormat,
                           const Standard_Size theSizeX,
                           const Standard_Size theSizeY,
                           const Standard_Size theSizeRowBytes = 0) Standard_OVERRIDE {
        (void )thePixelFormat;
        (void )theSizeX;
        (void )theSizeY;
        (void )theSizeRowBytes;
        Clear();
        return false;
    }

    //! Load image from file.
    ST_LOCAL bool Load(const StString& theFileName,
                       const StMIME& theMime,
                       uint8_t* theDataPtr = NULL, int theDataSize = 0);

        private:

    StHandle<StImageFile> myStImage;

};

#endif // __StImageOcct_h_
