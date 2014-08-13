/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
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
#include <StAV/StAVFrame.h>

// forward declarations
class StVideoQueue;
class StThread;

// define StHandle template specialization
ST_DEFINE_HANDLE(StVideoQueue, StAVPacketQueue);

#if(LIBAVCODEC_VERSION_INT < AV_VERSION_INT(54, 18, 100))
    #define ST_AV_OLDSYNC
#endif

/**
 * This is Video playback class (filled OpenGL textures)
 * which feeded with packets (StAVPacket),
 * so it also implements StAVPacketQueue.
 */
class StVideoQueue : public StAVPacketQueue {

        public:

    /**
     * @return true if GPU usage is enabled
     */
    ST_LOCAL bool toUseGpu() const {
        return myUseGpu;
    }

    /**
     * Setup GPU usage. Requires re-initialization to take effect!
     */
    ST_LOCAL void setUseGpu(const bool theToUseGpu) {
        myUseGpu = theToUseGpu;
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

    ST_LOCAL int64_t getVideoPktPts() const {
        return myVideoPktPts;
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
     * @return sterescopic information stored in file
     */
    ST_LOCAL StFormatEnum getStereoFormatFromStream() const {
        return myStFormatInStream;
    }

    /**
     * @return source format specified by user
     */
    ST_LOCAL StFormatEnum getStereoFormatByUser() const {
        return myStFormatByUser;
    }

    ST_LOCAL void setStereoFormatByUser(const StFormatEnum theSrcFormat) {
        myStFormatByUser = theSrcFormat;
    }

    /**
     * @return source format detected from file name
     */
    ST_LOCAL StFormatEnum getStereoFormatFromName() const {
        return myStFormatByName;
    }

    ST_LOCAL StVideoQueue(const StHandle<StGLTextureQueue>& theTextureQueue,
                          const StHandle<StVideoQueue>&     theMaster = StHandle<StVideoQueue>());
    ST_LOCAL virtual ~StVideoQueue();

    /**
     * Initialization function.
     * @param theFormatCtx pointer to video format context
     * @param theStreamId  stream id in video format context
     * @return true if no error
     */
    ST_LOCAL virtual bool init(AVFormatContext*   theFormatCtx,
                               const unsigned int theStreamId,
                               const StString&    theFileName);

    /**
     * Clean function.
     */
    ST_LOCAL virtual void deinit();

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
     * Initialize codec context.
     */
    ST_LOCAL bool initCodec(AVCodec* theCodec);

    /**
     * Initialize adapter over AVframe or perform to RGB conversion.
     */
    ST_LOCAL void prepareFrame(const StFormatEnum theSrcFormat);

    ST_LOCAL void pushFrame(const StImage&     theSrcDataLeft,
                            const StImage&     theSrcDataRight,
                            const StHandle<StStereoParams>& theStParams,
                            const StFormatEnum theSrcFormat,
                            const double       theSrcPTS);

        private:

    StHandle<StThread>         myThread;          //!< decoding loop thread
    StCondition                myDowntimeState;   //!< event to indicate downtime state
    StHandle<StGLTextureQueue> myTextureQueue;    //!< decoded frames queue

    StCondition                myHasDataState;
    StHandle<StVideoQueue>     myMaster;          //!< handle to Master decoding thread
    StHandle<StVideoQueue>     mySlave;           //!< handle to Slave  decoding thread

#if defined(_WIN32)
    AVCodec*                   myCodecDxva264;    //!< DXVA2 codec (decoding on GPU in Windows)
    AVCodec*                   myCodecDxvaWmv;    //!< DXVA2 codec (decoding on GPU in Windows)
    AVCodec*                   myCodecDxvaVc1;    //!< DXVA2 codec (decoding on GPU in Windows)
#elif defined(__APPLE__)
    AVCodec*                   myCodecVda;        //!< VDA codec (decoding on GPU in OS X)
#endif
    bool                       myUseGpu;          //!< activate decoding on GPU when possible

    StAVFrame                  myFrameRGB;        //!< frame, converted to RGB (soft)
    StImagePlane               myDataRGB;         //!< RGB buffer data (for swscale)
    SwsContext*                myToRgbCtx;        //!< software scaler context
    PixelFormat                myToRgbPixFmt;     //!< current swscale context - from pixel format
    bool                       myToRgbIsBroken;   //!< indicates broke swscale context - to RGB conversion is impossible

    StAVFrame                  myFrame;           //!< original decoded video frame
    StImage                    myDataAdp;         //!< buffer data adaptor
    AVDiscard                  myAvDiscard;       //!< discard parameter (to skip or not frames)

    double                     myFramePts;
    GLfloat                    myPixelRatio;      //!< pixel aspect ratio
    int                        myHParallax;       //!< horizontal parallax in pixels stored in metadata

    double                     myVideoClock;      //!< synchronization variable

    int64_t                    myVideoPktPts;

    StMutex                    myAudioClockMutex; //!< audio to video sync clock
    double                     myAudioClock;      //!< audio clock
    volatile int               myAudioDelayMSec;

    int64_t                    myFramesCounter;
    StImage                    myCachedFrame;
    bool                       myWasFlushed;

    volatile StFormatEnum      myStFormatByUser;  //!< source format specified by user
    volatile StFormatEnum      myStFormatByName;  //!< source format detected from file name
    volatile StFormatEnum      myStFormatInStream;//!< source format information retrieved from stream

};

#endif // __StVideoQueue_h_
