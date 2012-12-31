/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLTextureQueue_h_
#define __StGLTextureQueue_h_

#include <StThreads/StThreads.h> // threads header (mutexes, threads,...)
#include <StThreads/StFPSMeter.h>

#include "StGLQuadTexture.h"
#include "StGLTextureData.h"

/**
 * This is specialized class to maintain continuous frames queue.
 * Notice that it designed to be accessed from multiple threads but you should use it carefully!
 * One thread should be bound valid to OpenGL context (where the frames will be rendered)
 * and another thread(s) to continuously fill this queue.
 * Method stglUpdateStTextures() should be called each rendering call from GL thread to update textures.
 * Method push() should be used to fill in queue with new frames and stglSwapFB() to pop frame from queue
 * to display.
 */
class ST_LOCAL StGLTextureQueue {

        public:

    /**
     * Constructor.
     */
    StGLTextureQueue(const size_t theQueueSizeMax);

    /**
     * Destructor.
     */
    ~StGLTextureQueue();

    /**
     * @return quad texture.
     */
    StGLQuadTexture& getQTexture() {
        return myQTexture;
    }

    /**
     * @return input stream connection state.
     */
    bool hasConnectedStream() const {
        return myHasStream || !isEmpty();
    }

    /**
     * Setup input stream connection state.
     */
    void setConnectedStream(const bool theHasStream) {
        myHasStream = theHasStream;
    }

    /**
     * Function push stereo frame into queue.
     * This function called ONLY from video thread.
     * @param theSrcDataLeft  - first  INPUT data (Both or Left);
     * @param theSrcDataRight - second INPUT data (NULL or Right);
     * @param theStParams     - stereo parameters;
     * @param theSrcFormat    - source data format;
     * @param theSrcPTS       - PTS (presentation timestamp);
     * @return true on success.
     */
    bool push(const StImage&     theSrcDataLeft,
              const StImage&     theSrcDataRight,
              const StHandle<StStereoParams>& theStParams,
              const StFormatEnum theSrcFormat,
              const double       theSrcPTS);

    double getPlaybackFPS() {
        myMeterMutex.lock();
        const double fps     = myFPSMeter.getAverage();
        const bool isUpdated = myFPSMeter.isUpdated();
        myMeterMutex.unlock();
        if(isUpdated) {
            ST_DEBUG_LOG(
                + "Queue playback FPS " + fps
                + ", buffers: " + (myQueueSize + 1) + "/" + myQueueSizeMax
            );
        }
        return fps;
    }

    /**
     * Function called in loop from general GL draw loop
     * and do update quad texture data / state (display frame).
     * @return true on each full update.
     */
    bool stglUpdateStTextures(StGLContext& theCtx);

    size_t getSize() const {
        myMutexSize.lock();
            const size_t aResult = myQueueSize;
        myMutexSize.unlock();
        return aResult;
    }

    /**
     * @return true if queue is EMPTY.
     */
    bool isEmpty() const {
        myMutexSize.lock();
            const bool aResult = (myQueueSize == 0);
        myMutexSize.unlock();
        return aResult;
    }

    /**
     * @return true if queue is FULL.
     */
    bool isFull() const {
        myMutexSize.lock();
            const bool aResult = ((myQueueSize + 1) == myQueueSizeMax);
        myMutexSize.unlock();
        return aResult;
    }

    /**
     * @return presentation timestamp of currently shown frame (or -1 if none).
     */
    double getPTSCurr() const {
        myMutexSize.lock();
            const double aPts = (myHasStream || myQueueSize != 0)
                              ? myCurrPts : -1.0;
        myMutexSize.unlock();
        return aPts;
    }

    /**
     * @param thePts - next (front) stereo frame PTS (presentation timestamp);
     * @return false if next PTS not available.
     */
    bool popPTSNext(double& thePts) {
        bool aRes = false;
        myMutexPop.lock();
        myMutexPush.lock();
        myMutexSize.lock();
            // make sure front data is valid
            if(myQueueSize < 1) {
                aRes = false;
            } else {
                aRes = true;
                thePts = myDataFront->getPTS();
            }
        myMutexSize.unlock();
        myMutexPush.unlock();
        myMutexPop.unlock();
        return aRes;
    }

    /**
     * Send event to swap texture buffer Back->Front (increase swap counter).
     * @param theLimit - swap counter limit;
     * @return true if swap counter increased.
     */
    bool stglSwapFB(const size_t theLimit) {
        mySwapFBMutex.lock();
        if(theLimit == 0 || mySwapFBCount < theLimit) {
            ++mySwapFBCount;
            mySwapFBMutex.unlock();
            return true;
        }
        mySwapFBMutex.unlock();
        return false;
    }

    /**
     * Release unused memory as fast as possible.
     */
    void setCompressMemory(const bool theToCompress);

    /**
     * Function process TOTAL queue clean up.
     */
    void clear();

    /**
     * This function clean up only requested number of frames but prevents queue emptying.
     * At least one frame will remain in queue.
     */
    void drop(const size_t theCount);

    /**
     * Function used to get current showed source format.
     * At this moment function used just for stereo/mono recognizing.
     */
    int getSrcFormat() {
        myMutexSrcFormat.lock();
            // TODO (Kirill Gavrilov#4#) source format should be defined like front PTS to prevent early changes
            const int aSrcFrmt = myCurrSrcFormat;
        myMutexSrcFormat.unlock();
        return aSrcFrmt;
    }

    enum {
        SNAPSHOT_NO_NEW = 0,
        SNAPSHOT_SUCCESS = 1,
    };

    int getSnapshot(StImage* theOutDataLeft,
                    StImage* theOutDataRight,
                    bool     theToForce = false);

        private:

    enum {
        SWAPONREADY_NOTHING = 0,
        SWAPONREADY_SWAPPED = 1,
        SWAPONREADY_WAITLIM = 2,
    };

        private:

    int swapFBOnReady(StGLContext& theCtx);

        private:

    StMutex          myMutexPop;
    StGLTextureData* myDataFront;      //!< queue pointer
    StGLTextureData* myDataSnap;       //!< snapshot pointer
    StMutex          myMutexPush;
    StGLTextureData* myDataBack;       //!< queue pointer

    mutable StMutex  myMutexSize;
    size_t           myQueueSize;
    size_t           myQueueSizeMax;

    StGLQuadTexture  myQTexture;       //!< quad stereo texture

    StMutex          mySwapFBMutex;
    size_t           mySwapFBCount;

    StMutex          myMeterMutex;
    StFPSMeter       myFPSMeter;

    StMutex          myMutexSrcFormat;
    int              myCurrSrcFormat;  //!< current source format

    double           myCurrPts;

    StEvent          myNewShotEvent;
    bool             myIsInUpdTexture; //!< private bools for plugin thread
    bool             myIsReadyToSwap;
    bool             myToCompress;     //!< release unused memory as fast as possible
    volatile bool    myHasStream;      //!< flag indicates that some stream connected to this queue

};

#endif //__StGLTextureQueue_h_
