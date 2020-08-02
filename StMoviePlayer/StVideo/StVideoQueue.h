/**
 * Copyright © 2009-2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * StMoviePlayer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StMoviePlayer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StVideoQueue_h_
#define __StVideoQueue_h_

#include <StGLStereo/StGLTextureQueue.h>

#include "StAVPacketQueue.h"
#include <StAV/StAVImage.h>

// forward declarations
class StVideoQueue;
class StThread;

/**
 * Interface defining HWAccel context.
 */
class StHWAccelContext {

        public:

    /**
     * Initialize the context.
     */
    virtual bool create(StVideoQueue& theVideo) = 0;

    /**
     * Destructor.
     */
    virtual ~StHWAccelContext() {}

    /**
     * Return true if context has been successfully initialized.
     */
    virtual bool isValid() const = 0;

    /**
     * Create decoder.
     */
    virtual bool decoderCreate(StVideoQueue&   theVideo,
                               AVCodecContext* theCodecCtx) = 0;

    /**
     * Release decoder.
     */
    virtual void decoderDestroy(AVCodecContext* theCodecCtx) = 0;

    /**
     * AVFrame initialization callback.
     */
    virtual int getFrameBuffer(StVideoQueue& theVideo,
                               AVFrame*      theFrame) = 0;

    /**
     * Fetch decoded results into specified frame.
     */
    virtual bool retrieveFrame(StVideoQueue& theVideo,
                               AVFrame*      theFrame) = 0;

};

// define StHandle template specialization
ST_DEFINE_HANDLE(StVideoQueue, StAVPacketQueue);

#if(LIBAVCODEC_VERSION_INT < AV_VERSION_INT(54, 18, 100)) || defined(ST_LIBAV_FORK)
    #define ST_AV_OLDSYNC
#endif

/**
 * This is Video playback class (filled OpenGL textures)
 * which feeded with packets (StAVPacket),
 * so it also implements StAVPacketQueue.
 */
class StVideoQueue : public StAVPacketQueue {

        public:

    AVCodecID CodecIdH264;
    AVCodecID CodecIdHEVC;
    AVCodecID CodecIdMPEG2;
    AVCodecID CodecIdWMV3;
    AVCodecID CodecIdVC1;
    AVCodecID CodecIdJpeg2K;

        public:

    /**
     * @return true if GPU usage is enabled
     */
    ST_LOCAL bool toUseGpu() const {
        return myUseGpu;
    }

    /**
     * Return true if GPU usage has been requested but failed (e.g. decoder should be reinitialized).
     */
    ST_LOCAL bool isGpuFailed() const {
        return myIsGpuFailed;
    }

    /**
     * Setup GPU usage. Requires re-initialization to take effect!
     */
    ST_LOCAL void setUseGpu(const bool theToUseGpu,
                            const bool theIsGpuFailed = false) {
        myUseGpu      = theToUseGpu;
        myIsGpuFailed = theIsGpuFailed;
    }

    /**
     * Setup OpenJPEG usage. Requires re-initialization to take effect!
     */
    ST_LOCAL void setUseOpenJpeg(const bool theToUseOpenJpeg) {
        myUseOpenJpeg = theToUseOpenJpeg;
    }

    ST_LOCAL bool isInDowntime() {
        return myDowntimeState.check();
    }

    ST_LOCAL void setSlave(const StHandle<StVideoQueue>& theSlave) {
        mySlave = theSlave;
    }

    ST_LOCAL StImage* waitData(double& thePts) {
        myHasDataState.wait();
        if(myDataAdp.isNull()) {
            return NULL;
        }
        thePts = myFramePts;
        return &myDataAdp;
    }

    ST_LOCAL void unlockData() {
        myHasDataState.reset();
    }

    ST_LOCAL void setAClock(const double thePts) {
        myAudioClockMutex.lock();
        myAudioClock = thePts;
        myAudioClockMutex.unlock();
    }

    ST_LOCAL double getAClock() {
        myAudioClockMutex.lock();
        const double aPts = myAudioClock;
        myAudioClockMutex.unlock();
        return aPts;
    }

    ST_LOCAL void setAudioDelay(const int theDelayMSec) {
        myAudioDelayMSec = theDelayMSec;
    }

    ST_LOCAL StCString getPixelFormatString() const {
        return stAV::PIX_FMT::getString(myCodecCtx->pix_fmt);
    }

    /**
     * @return stereoscopic information stored in file
     */
    ST_LOCAL StFormat getStereoFormatFromStream() const {
        return myStFormatInStream;
    }

    /**
     * @return source format specified by user
     */
    ST_LOCAL StFormat getStereoFormatByUser() const {
        return myStFormatByUser;
    }

    ST_LOCAL void setStereoFormatByUser(const StFormat theSrcFormat) {
        myStFormatByUser = theSrcFormat;
    }

    /**
     * @return source format detected from file name
     */
    ST_LOCAL StFormat getStereoFormatFromName() const {
        return myStFormatByName;
    }

    /**
     * Set theater mode.
     */
    ST_LOCAL void setTheaterMode(bool theIsTheater) {
        myIsTheaterMode = theIsTheater;
    }

    /**
     * Stick to panorama 360 mode.
     */
    ST_LOCAL void setStickPano360(bool theToStick) {
        myToStickPano360 = theToStick;
    }

    /**
     * Set if JPS file should be read as Left/Right (TRUE) of as Right/Left (FALSE).
     */
    ST_LOCAL void setSwapJPS(bool theToSwap) { myToSwapJps = theToSwap; }

    ST_LOCAL StVideoQueue(const StHandle<StGLTextureQueue>& theTextureQueue,
                          const StHandle<StVideoQueue>&     theMaster = StHandle<StVideoQueue>());
    ST_LOCAL virtual ~StVideoQueue();

    /**
     * Return codec type.
     */
    ST_LOCAL virtual AVMediaType getCodecType() const ST_ATTR_OVERRIDE { return AVMEDIA_TYPE_VIDEO; }

    /**
     * Initialization function.
     * @param theFormatCtx pointer to video format context
     * @param theStreamId  stream id in video format context
     * @return true if no error
     */
    ST_LOCAL bool init(AVFormatContext*   theFormatCtx,
                       const unsigned int theStreamId,
                       const StString&    theFileName,
                       const StHandle<StStereoParams>& theNewParams);

    /**
     * Clean function.
     */
    ST_LOCAL virtual void deinit() ST_ATTR_OVERRIDE;

#ifdef ST_AV_OLDSYNC
    ST_LOCAL void syncVideo(AVFrame* srcFrame, double* pts);
#endif

    /**
     * Main decoding loop.
     * Give packets from queue, decode them and push to stereo textures queue for playback.
     */
    ST_LOCAL void decodeLoop();

    /**
     * @return PAR
     */
    ST_LOCAL GLfloat getPixelRatio() const {
        return myPixelRatio;
    }

    /**
     * @return source frame width
     */
    ST_LOCAL int sizeX() const {
        return (myCodecCtx != NULL) ? myCodecCtx->width : 0;
    }

    /**
     * @return source frame height
     */
    ST_LOCAL int sizeY() const {
        return (myCodecCtx != NULL) ? myCodecCtx->height : 0;
    }

    /**
     * @return frame coded width (before cropping)
     */
    ST_LOCAL int getCodedSizeX() const {
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 0, 0))
        return (myCodecCtx != NULL) ? myCodecCtx->coded_width : 0;
    #else
        return (myCodecCtx != NULL) ? myCodecCtx->width : 0;
    #endif
    }

    /**
     * @return frame coded height (before cropping)
     */
    ST_LOCAL int getCodedSizeY() const {
    #if(LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 0, 0))
        return (myCodecCtx != NULL) ? myCodecCtx->coded_height : 0;
    #else
        return (myCodecCtx != NULL) ? myCodecCtx->height : 0;
    #endif
    }

    ST_LOCAL StHandle<StGLTextureQueue>& getTextureQueue() {
        return myTextureQueue;
    }

    ST_LOCAL double getPts() const {
        return myTextureQueue->getPTSCurr();
    }

        private:

    /**
     * Select frame format from the list.
     */
    ST_LOCAL static AVPixelFormat stGetFrameFormat(AVCodecContext*      theCodecCtx,
                                                   const AVPixelFormat* theFormats) {
        StVideoQueue* aVideoQueue = (StVideoQueue* )theCodecCtx->opaque;
        return aVideoQueue->getFrameFormat(theFormats);
    }

    /**
     * Frame buffer allocation callback (deprecated form).
     */
    ST_LOCAL static int stGetFrameBuffer1(AVCodecContext* theCodecCtx,
                                          AVFrame*        theFrame) {
        StVideoQueue* aVideoQueue = (StVideoQueue* )theCodecCtx->opaque;
        return aVideoQueue->getFrameBuffer(theFrame, 0);
    }

    /**
     * Frame buffer allocation callback (wrapper).
     */
    ST_LOCAL static int stGetFrameBuffer2(AVCodecContext* theCodecCtx,
                                          AVFrame*        theFrame,
                                          int             theFlags) {
        StVideoQueue* aVideoQueue = (StVideoQueue* )theCodecCtx->opaque;
        return aVideoQueue->getFrameBuffer(theFrame, theFlags);
    }

        private:

    /**
     * Initialize codec context.
     */
    ST_LOCAL bool initCodec(AVCodec*   theCodec,
                            const bool theToUseGpu);

    /**
     * Select frame format from the list.
     */
    ST_LOCAL AVPixelFormat getFrameFormat(const AVPixelFormat* theFormats);

    /**
     * Frame buffer allocation callback.
     */
    ST_LOCAL int getFrameBuffer(AVFrame* theFrame,
                                int      theFlags);

    /**
     * Initialize hardware accelerated decoder.
     */
    ST_LOCAL bool hwaccelInit();

private:

    /**
     * Detect 720in1080 streams with cropping information
     */
    ST_LOCAL bool check720in1080() const {
        return (sizeX() == 1280) && (sizeY() == 720)
            && (getCodedSizeX() == 1920)
            && (getCodedSizeY() == 1080 || getCodedSizeY() == 1088);
    }

    ST_LOCAL bool decodeFrame(const StHandle<StAVPacket>& thePacket,
                              bool& theToSendPacket,
                              bool& theIsStarted,
                              StString& theTagValue,
                              double& theAverageDelaySec,
                              double& thePrevPts);

    /**
     * Initialize adapter over AVframe or perform to RGB conversion.
     */
    ST_LOCAL void prepareFrame(const StFormat theSrcFormat);

    ST_LOCAL void pushFrame(const StImage&     theSrcDataLeft,
                            const StImage&     theSrcDataRight,
                            const StHandle<StStereoParams>& theStParams,
                            const StFormat     theSrcFormat,
                            const StCubemap    theCubemapFormat,
                            const double       theSrcPTS);

        private:

    StHandle<StThread>         myThread;          //!< decoding loop thread
    StCondition                myDowntimeState;   //!< event to indicate downtime state
    StHandle<StGLTextureQueue> myTextureQueue;    //!< decoded frames queue

    StCondition                myHasDataState;
    StHandle<StVideoQueue>     myMaster;          //!< handle to Master decoding thread
    StHandle<StVideoQueue>     mySlave;           //!< handle to Slave  decoding thread

    StHandle<StHWAccelContext> myHWAccelCtx;
#if defined(__ANDROID__)
    AVCodec*                   myCodecH264HW;     //!< h264 decoder using dedicated hardware (Android Media Codec)
    AVCodec*                   myCodecHevcHW;     //!< hevc decoder using dedicated hardware
    AVCodec*                   myCodecVp9HW;      //!< vp9  decoder using dedicated hardware
#endif
    AVCodec*                   myCodecOpenJpeg;   //!< libopenjpeg decoder
    bool                       myUseGpu;          //!< activate decoding on GPU when possible
    bool                       myIsGpuFailed;     //!< flag indicating that GPU decoder can not handle input data
    bool                       myUseOpenJpeg;     //!< use OpenJPEG (libopenjpeg) instead of built-in jpeg2000 decoder

    StAVFrame                  myFrameRGB;        //!< frame, converted to RGB (soft)
    StImagePlane               myDataRGB;         //!< RGB buffer data (for swscale)
    SwsContext*                myToRgbCtx;        //!< software scaler context
    AVPixelFormat              myToRgbPixFmt;     //!< current swscale context - from pixel format
    bool                       myToRgbIsBroken;   //!< indicates broke swscale context - to RGB conversion is impossible

    StAVFrame                  myFrame;           //!< original decoded video frame
    StHandle<StAVFrameCounter> myFrameBufRef;
    StImage                    myDataAdp;         //!< buffer data adaptor
    AVDiscard                  myAvDiscard;       //!< discard parameter (to skip or not frames)

    double                     myFramePts;
    GLfloat                    myPixelRatio;      //!< pixel aspect ratio
    int                        myHParallax;       //!< horizontal parallax in pixels stored in metadata
    int                        myRotateDeg;       //!< rotate angle in degrees

    double                     myVideoClock;      //!< synchronization variable

    int64_t                    myVideoPktPts;

    StMutex                    myAudioClockMutex; //!< audio to video sync clock
    double                     myAudioClock;      //!< audio clock
    volatile int               myAudioDelayMSec;

    int64_t                    myFramesCounter;
    StImage                    myCachedFrame;
    StImage                    myEmptyImage;
    bool                       myWasFlushed;

    volatile StFormat          myStFormatByUser;  //!< source format specified by user
    volatile StFormat          myStFormatByName;  //!< source format detected from file name
    volatile StFormat          myStFormatInStream;//!< source format information retrieved from stream
    volatile bool              myIsTheaterMode;   //!< flag indicating theater mode
    volatile bool              myToStickPano360;  //!< stick to panorama 360 mode
    volatile bool              myToSwapJps;       //!< read JPS as Left/Right instead of Right/Left

};

#endif // __StVideoQueue_h_
