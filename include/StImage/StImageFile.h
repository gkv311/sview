/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StImageFile_h_
#define __StImageFile_h_

#include <StTemplates/StHandle.h>

#include "StImage.h"

class StMIME;

class ST_LOCAL StImageFile : public StImage {

        public:

    typedef enum tagImageType {
        ST_TYPE_NONE = 0,
        ST_TYPE_PNG,
        ST_TYPE_PNS,
        ST_TYPE_JPEG,
        ST_TYPE_JPS,
        ST_TYPE_MPO,
        ST_TYPE_EXR,
        ST_TYPE_ICO,
        ST_TYPE_PSD,
        ST_TYPE_HDR, //!< Radiance High Dynamic Range - .hdr extension
        ST_TYPE_WEBP,
        ST_TYPE_WEBPLL,
    } ImageType;

    typedef enum tagImageClass {
        ST_LIBAV,
        ST_DEVIL,
        ST_FREEIMAGE,
        ST_WEBP,
    } ImageClass;

        private:

    StString myStateDescr;

        public:

    static ImageClass imgLibFromString(const StString& thePreferred);
    static StString imgLibToString(const ImageClass thePreferred);

    /**
     * Guess the image type for the file (file extension in simplest case).
     * If specified MIME type is not empty than it may override detection.
     * @param theFileName (const StString& ) - path to the image;
     * @param theMIMEType (const StMIME&   ) - image MIME type, may be empty.
     */
    static ImageType guessImageType(const StString& theFileName,
                                    const StMIME&   theMIMEType);

    static StHandle<StImageFile> create(const StString& thePreferred, ImageType theImgType = ST_TYPE_NONE);
    static StHandle<StImageFile> create(ImageClass thePreferred = ST_LIBAV, ImageType theImgType = ST_TYPE_NONE);

    /**
     * Empty constructor.
     */
    StImageFile() : StImage(), myStateDescr() {}
    virtual ~StImageFile() {}

    /**
     * @return the error description occured on load/save operations.
     */
    const StString& getState() const {
        return myStateDescr;
    }

    StString& changeState() {
        return myStateDescr;
    }

    void setState(const StString& theDescr = StString()) {
        myStateDescr = theDescr;
    }

    /**
     * Returns the number of frames in multi-page image.
     */
    ///virtual size_t getFramesCount() const = 0;

    /**
     * This virtual function should be implemented by inheritors.
     * @param theFilePath (const StString& ) path to the file;
     * @param theImageType (int ) image type, should be set for files with undefined extension (mpo/jps/pns...);
     * @param theDataPtr (uint8_t* ) data in the memory, image will be read from file if this data empty;
     * @param theDataSize (int ) size of data in memory;
     * @return true on success.
     */
    virtual bool load(const StString& theFilePath,
                      ImageType theImageType = ST_TYPE_NONE,
                      uint8_t* theDataPtr = NULL, int theDataSize = 0) = 0;

    /**
     * Close the file after it was opened with read() method.
     * This will probably invalidate current image data because it often
     * just a wrapper over native library storage pointers.
     */
    virtual void close() = 0;

    /**
     * This virtual function should be implemented by inheritors.
     * @param theFilePath (const StString& ) path to save the file;
     * @param theImageType (int ) image type;
     * @return true on success.
     */
    virtual bool save(const StString& theFilePath,
                      ImageType theImageType) = 0;

    /**
     * Resize image.
     */
    virtual bool resize(size_t theSizeX, size_t theSizeY) = 0;

};

#endif //__StImageFile_h_
