/**
 * Copyright © 2011-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StAVImage_h_
#define __StAVImage_h_

#include <StImage/StImageFile.h>
#include <StAV/StAVFrame.h>

struct AVInputFormat;
struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;

// define StHandle template specialization
class StAVImage;
ST_DEFINE_HANDLE(StAVImage, StImageFile);

class StAVFrameCounter;
ST_DEFINE_HANDLE(StAVFrameCounter, StBufferCounter);

/**
 * This class implements image load/save operation using libav* libraries.
 */
class StAVImage : public StImageFile {

        public:

    /**
     * Should be called at application start.
     * Short-link to the stLibAV::init().
     */
    ST_CPPEXPORT static bool init();

    /**
     * Resize image using swscale library from FFmpeg.
     * There are several restriction:
     * - Destination image should have the same format (this method is for scaling, not conversion).
     * - Memory should be properly aligned.
     */
    ST_CPPEXPORT static bool resize(const StImage& theImageFrom,
                                    StImage&       theImageTo);

    ST_CPPEXPORT static bool resizePlane(const StImagePlane& theImageFrom,
                                         StImagePlane&       theImageTo);

        public:

    /**
     * Initialize empty image.
     */
    ST_CPPEXPORT StAVImage();

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StAVImage();

    /**
     * Create new instance.
     */
    virtual StHandle<StImageFile> createEmpty() const ST_ATTR_OVERRIDE { return new StAVImage(); }

    /**
     * Close currently opened image context and release memory.
     */
    ST_CPPEXPORT virtual void close() ST_ATTR_OVERRIDE;

    /**
     * Decode image from specified file or memory pointer.
     */
    ST_CPPEXPORT virtual bool loadExtra(const StString& theFilePath,
                                        ImageType       theImageType,
                                        uint8_t*        theDataPtr,
                                        int             theDataSize,
                                        bool            theIsOnlyRGB) ST_ATTR_OVERRIDE;

    /**
     * Save image to specified path.
     */
    ST_CPPEXPORT virtual bool save(const StString& theFilePath,
                                   ImageType       theImageType,
                                   StFormat        theSrcFormat = StFormat_AUTO) ST_ATTR_OVERRIDE;

        private:

    ST_LOCAL static int getAVPixelFormat(const StImage& theImage);

    /**
     * Close currently opened image context and release memory.
     */
    ST_LOCAL void closeAvCtx();

        private:

    AVInputFormat*   myImageFormat; //!< image format
    AVFormatContext* myFormatCtx;   //!< file context
    AVCodecContext*  myCodecCtx;    //!< codec context
    AVCodec*         myCodec;       //!< codec
    StAVFrame        myFrame;

};

/**
 * Pass reference-counted AVFrame (e.g. AVBuffer) into StImage without data copying.
 */
class StAVFrameCounter : public StBufferCounter {

        public:

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StAVFrameCounter();

    /**
     * Create the new reference (e.g. increment counter).
     * If theOther has the same type, than the ref counter will be reused.
     * Otherwise then new counter will be allocated.
     */
    ST_CPPEXPORT virtual void createReference(StHandle<StBufferCounter>& theOther) const;

    /**
     * Release current reference.
     */
    ST_CPPEXPORT virtual void releaseReference();

    /**
     * Release reference counter.
     */
    ST_CPPEXPORT virtual ~StAVFrameCounter();

    /**
     * Initialize a proxy reference.
     */
    ST_CPPEXPORT void moveReferenceFrom(AVFrame* theFrame);

        private:

    AVFrame* myFrame;   //!< frame
    bool     myIsProxy; //!< proxy reference to be moved, not copied

};

#endif // __StAVImage_h_
