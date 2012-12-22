/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
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

// forward declarations
class StVideoQueue;

// define StHandle template specialization
ST_DEFINE_HANDLE(StVideoQueue, StAVPacketQueue);

/**
 * This is Video playback class (filled OpenGL textures)
 * which feeded with packets (StAVPacket),
 * so it also implements StAVPacketQueue.
 */
class ST_LOCAL StVideoQueue : public StAVPacketQueue {

        private:

    StHandle<StThread> myThread; //!< decoding loop thread
    StEvent evDowntime;

    StHandle<StGLTextureQueue> myTextureQueue; // decoded frames queue

    StEvent evHasData;
    StHandle<StVideoQueue> myMaster; //!< handle to Master decoding thread
    StHandle<StVideoQueue> mySlave;  //!< handle to Slave  decoding thread

    AVDiscard          avDiscard; // discard parameter (to skip or not frames)

    AVFrame*               frame; // original VIDEO frame
    AVFrame*            frameRGB; // frame, converted to RGB (soft)
    uint8_t*           bufferRGB; // buffer
    StImage              dataAdp; // buffer data adaptor
    SwsContext*        pToRgbCtx; // software scaler context
    double            myFramePts;
    GLfloat           pixelRatio; // pixel aspect ratio

    double            videoClock; // synchronization variable

    int64_t          videoPktPts;

    StMutex          aClockMutex; // audio to video sync clock
    double                aClock; // audio clock

    int64_t      myFramesCounter;
    StImage        myCachedFrame;
    bool            myWasFlushed;

    StFormatEnum     mySrcFormat; // source format
    StFormatEnum mySrcFormatInfo; // source format information retrieved from stream

        public:

    bool isInDowntime() {
        return evDowntime.check();
    }

    void setSlave(const StHandle<StVideoQueue>& theSlave) {
        mySlave = theSlave;
    }

    StImage* waitData(double& thePts) {
        evHasData.wait();
        if(dataAdp.isNull()) {
            return NULL;
        }
        thePts = myFramePts;
        return &dataAdp;
    }

    void unlockData() {
        evHasData.reset();
    }

    int64_t getVideoPktPts() const {
        return videoPktPts;
    }

    void setAClock(const double thePts) {
        aClockMutex.lock();
            aClock = thePts;
        aClockMutex.unlock();
    }

    double getAClock() {
        aClockMutex.lock();
            double aPts = aClock;
        aClockMutex.unlock();
        return aPts;
    }

    StString getPixelFormatString() const {
        return stLibAV::PIX_FMT::getString(myCodecCtx->pix_fmt);
    }

    StFormatEnum getSrcFormat() const {
        // TODO (Kirill Gavrilov#4#) not thread safe
        return mySrcFormat;
    }

    void setSrcFormat(const StFormatEnum theSrcFormat) {
        // TODO (Kirill Gavrilov#4#) not thread safe
        mySrcFormat = theSrcFormat;
    }

    StVideoQueue(const StHandle<StGLTextureQueue>& theTextureQueue,
                 const StHandle<StVideoQueue>&     theMaster = StHandle<StVideoQueue>());
    virtual ~StVideoQueue();

    /**
     * Initialization function.
     * @param theFormatCtx (AVFormatContext* )  - pointer to video format context;
     * @param theStreamId (const unsigned int ) - stream id in video format context;
     * @return true if no error.
     */
    virtual bool init(AVFormatContext*   theFormatCtx,
                      const unsigned int theStreamId);

    /**
     * Clean function.
     */
    virtual void deinit();

    void syncVideo(AVFrame* srcFrame, double* pts);

    /**
     * Main decoding loop.
     * Give packets from queue, decode them and push to stereo textures queue for playback.
     */
    void decodeLoop();

    /**
     * @return PAR.
     */
    GLfloat getPixelRatio() const {
        return pixelRatio;
    }

    /**
     * @return sizeX (int ) - source frame width.
     */
    int sizeX() const {
        return (myCodecCtx != NULL) ? myCodecCtx->width : 0;
    }

    /**
     * @return sizeY (int ) - source frame height.
     */
    int sizeY() const {
        return (myCodecCtx != NULL) ? myCodecCtx->height : 0;
    }

    StHandle<StGLTextureQueue>& getTextureQueue() {
        return myTextureQueue;
    }

    double getPts() const {
        return myTextureQueue->getPTSCurr();
    }

        private:

    void pushFrame(const StImage&     theSrcDataLeft,
                   const StImage&     theSrcDataRight,
                   const StHandle<StStereoParams>& theStParams,
                   const StFormatEnum theSrcFormat,
                   const double       theSrcPTS);

};

#endif //__StVideoQueue_h_
