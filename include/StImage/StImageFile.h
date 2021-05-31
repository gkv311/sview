/**
 * Copyright © 2011-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StImageFile_h_
#define __StImageFile_h_

#include <StTemplates/StHandle.h>
#include <StStrings/StDictionary.h>
#include <StGLStereo/StFormatEnum.h>

#include "StImage.h"

class StMIME;

// define StHandle template specialization
class StImageFile;
ST_DEFINE_HANDLE(StImageFile, StImage);

class StImageFileCounter;
ST_DEFINE_HANDLE(StImageFileCounter, StBufferCounter);

/**
 * Interface extending StImage with load/save capabilities.
 */
class StImageFile : public StImage {

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
        ST_TYPE_DDS,
    } ImageType;

    typedef enum tagImageClass {
        ST_LIBAV,
        ST_DEVIL,
        ST_FREEIMAGE,
        ST_WEBP,
        ST_STB,
    } ImageClass;

        public:

    ST_CPPEXPORT static ImageClass imgLibFromString(const StString&  thePreferred);
    ST_CPPEXPORT static StString   imgLibToString  (const ImageClass thePreferred);

    /**
     * Guess the image type for the file (file extension in simplest case).
     * If specified MIME type is not empty than it may override detection.
     * @param theFileName path to the image
     * @param theMIMEType image MIME type, may be empty
     */
    ST_CPPEXPORT static ImageType guessImageType(const StString& theFileName,
                                                 const StMIME&   theMIMEType);

    ST_CPPEXPORT static StHandle<StImageFile> create(const StString& thePreferred,
                                                     ImageType       theImgType = ST_TYPE_NONE);
    ST_CPPEXPORT static StHandle<StImageFile> create(ImageClass      thePreferred = ST_LIBAV,
                                                     ImageType       theImgType = ST_TYPE_NONE);

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StImageFile();
    ST_CPPEXPORT virtual ~StImageFile();

    /**
     * @return the error description occured on load/save operations.
     */
    ST_LOCAL const StString& getState() const {
        return myStateDescr;
    }

    ST_LOCAL StString& changeState() {
        return myStateDescr;
    }

    ST_LOCAL void setState(const StString& theDescr = StString()) {
        myStateDescr = theDescr;
    }

    /**
     * Return metadata associated with image.
     */
    ST_LOCAL const StDictionary& getMetadata() const {
        return myMetadata;
    }

    /**
     * Assign new metadata.
     */
    ST_LOCAL void setMetadata(const StDictionary& theDict) {
        myMetadata = theDict;
    }

    /**
     * Return stereoscopic format stored in the file or StFormat_AUTO if undefined.
     */
    ST_LOCAL StFormat getFormat() const {
        return mySrcFormat;
    }

    /**
     * Return panorama format stored in the file or StPanorama_OFF if undefined.
     */
    ST_LOCAL StPanorama getPanoramaFormat() const { return mySrcPanorama; }

    /**
     * Returns the number of frames in multi-page image.
     */
    ///virtual size_t getFramesCount() const = 0;

    /**
     * Load the image from specified source.
     * @param theFilePath  path to the file
     * @param theImageType image type, should be set for files with undefined extension (mpo/jps/pns...)
     * @param theDataPtr   data in the memory, image will be read from file if this data empty
     * @param theDataSize  size of data in memory
     * @return true on success
     */
    ST_CPPEXPORT bool load(const StString& theFilePath,
                           ImageType theImageType = ST_TYPE_NONE,
                           uint8_t* theDataPtr = NULL, int theDataSize = 0);

    /**
     * This virtual function should be implemented by inheritors.
     * @param theFilePath  path to the file
     * @param theImageType image type, should be set for files with undefined extension (mpo/jps/pns...)
     * @param theDataPtr   data in the memory, image will be read from file if this data empty
     * @param theDataSize  size of data in memory
     * @param theIsOnlyRGB option to convert YUV image data into RGB format
     * @return true on success
     */
    virtual bool loadExtra(const StString& theFilePath,
                           ImageType       theImageType,
                           uint8_t*        theDataPtr,
                           int             theDataSize,
                           bool            theIsOnlyRGB) = 0;

    /**
     * Close the file after it was opened with read() method.
     * This will probably invalidate current image data because it often
     * just a wrapper over native library storage pointers.
     */
    virtual void close() = 0;

    /**
     * This virtual function should be implemented by inheritors.
     * @param theFilePath  path to save the file
     * @param theImageType image type
     * @param theSrcFormat stereo format - might be stored as metadata
     * @return true on success
     */
    virtual bool save(const StString& theFilePath,
                      ImageType       theImageType,
                      StFormat        theSrcFormat) = 0;

    /**
     * Create new instance of this class.
     */
    virtual StHandle<StImageFile> createEmpty() const = 0;

        protected:

    StDictionary myMetadata;
    StString     myStateDescr;
    StFormat     mySrcFormat;
    StPanorama   mySrcPanorama;

};

/**
 * Define reference-counted StImageFile buffer for StImage.
 */
class StImageFileCounter : public StBufferCounter {

        public:

    /**
     * Empty constructor.
     */
    ST_LOCAL StImageFileCounter() {}

    /**
     * Main constructor.
     */
    ST_LOCAL StImageFileCounter(const StHandle<StImage>& theImage) : myImageFile(theImage) {}

    /**
     * Create the new reference (e.g. increment counter).
     * If theOther has the same type, than the ref counter will be reused.
     * Otherwise then new counter will be allocated.
     */
    ST_CPPEXPORT virtual void createReference(StHandle<StBufferCounter>& theOther) const ST_ATTR_OVERRIDE;

    /**
     * Release current reference.
     */
    ST_CPPEXPORT virtual void releaseReference() ST_ATTR_OVERRIDE;

    /**
     * Release reference counter.
     */
    ST_CPPEXPORT virtual ~StImageFileCounter();

        private:

    StHandle<StImage> myImageFile;

};

#endif //__StImageFile_h_
