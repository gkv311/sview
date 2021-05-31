/**
 * Copyright © 2009-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLTextureQueue_h_
#define __StGLTextureQueue_h_

#include <StThreads/StCondition.h>
#include <StThreads/StFPSMeter.h>
#include <StThreads/StMutex.h>

#include <StGL/StGLDeviceCaps.h>

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
class StGLTextureQueue {

        public:

    /**
     * Constructor.
     */
    ST_CPPEXPORT StGLTextureQueue(const size_t theQueueSizeMax);

    /**
     * Destructor.
     */
    ST_CPPEXPORT ~StGLTextureQueue();

    /**
     * Get device capabilities.
     */
    ST_LOCAL const StGLDeviceCaps& getDeviceCaps() const { return myDeviceCaps; }

    /**
     * Set device capabilities.
     */
    ST_LOCAL void setDeviceCaps(const StGLDeviceCaps& theCaps) {
        myMutexPush.lock();
        myDeviceCaps = theCaps;
        myMutexPush.unlock();
    }

    /**
     * @return quad texture
     */
    ST_LOCAL StGLQuadTexture& getQTexture() {
        return myQTexture;
    }

    /**
     * Return texture streaming parameters.
     */
    ST_LOCAL StGLTextureUploadParams& getUploadParams() { return *myUploadParams; }

    /**
     * @return input stream connection state
     */
    ST_LOCAL bool hasConnectedStream() const {
        return myHasStream || !isEmpty();
    }

    /**
     * Setup input stream connection state.
     */
    ST_LOCAL void setConnectedStream(const bool theHasStream) {
        myHasStream = theHasStream;
    }

    /**
     * Function push stereo frame into queue.
     * This function called ONLY from video thread.
     * @param theSrcDataLeft  first  INPUT data (Both or Left)
     * @param theSrcDataRight second INPUT data (NULL or Right)
     * @param theStParams     stereo parameters
     * @param theSrcFormat    source data format
     * @param theSrcCubemap   format of cubemap
     * @param theSrcPTS       PTS (presentation timestamp)
     * @return true on success
     */
    ST_CPPEXPORT bool push(const StImage&     theSrcDataLeft,
                           const StImage&     theSrcDataRight,
                           const StHandle<StStereoParams>& theStParams,
                           const StFormat     theSrcFormat,
                           const StCubemap    theSrcCubemap,
                           const double       theSrcPTS);

    /**
     * Retrieve queue statistics.
     */
    ST_LOCAL inline void getQueueInfo(int&    theQueued,
                                      int&    theQueueLen,
                                      double& theFps) {
        myMeterMutex.lock();
        if(myHasStream) {
            theQueued   = int(myQueueSize + 1);
            theQueueLen = int(myQueueSizeMax);
            theFps      = myFPSMeter.getAverage();
        } else {
            theQueued   = 0;
            theQueueLen = 0;
            theFps      = -1.0;
        }
        const bool isUpdated = myFPSMeter.isUpdated();
        myMeterMutex.unlock();
        if(isUpdated) {
            ST_DEBUG_LOG("Queue playback FPS " + theFps + ", buffers: " + theQueued + "/" + theQueueLen);
        }
    }

    /**
     * Function called in loop from general GL draw loop
     * and do update quad texture data / state (display frame).
     * @return true on each full update
     */
    ST_CPPEXPORT bool stglUpdateStTextures(StGLContext& theCtx);

    ST_LOCAL size_t getSize() const {
        myMutexSize.lock();
            const size_t aResult = myQueueSize;
        myMutexSize.unlock();
        return aResult;
    }

    /**
     * @return true if queue is EMPTY.
     */
    ST_LOCAL bool isEmpty() const {
        myMutexSize.lock();
            const bool aResult = (myQueueSize == 0);
        myMutexSize.unlock();
        return aResult;
    }

    /**
     * @return true if queue is FULL.
     */
    ST_LOCAL bool isFull() const {
        myMutexSize.lock();
            const bool aResult = ((myQueueSize + 1) == myQueueSizeMax);
        myMutexSize.unlock();
        return aResult;
    }

    /**
     * @return presentation timestamp of currently shown frame (or -1 if none).
     */
    ST_LOCAL double getPTSCurr() const {
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
    ST_LOCAL bool popPTSNext(double& thePts) {
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
    ST_LOCAL bool stglSwapFB(const size_t theLimit) {
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
    ST_CPPEXPORT void setCompressMemory(const bool theToCompress);

    /**
     * Function process TOTAL queue clean up.
     */
    ST_CPPEXPORT void clear();

    /**
     * This function clean up only requested number of frames but prevents queue emptying.
     * At least one frame will remain in queue.
     */
    ST_CPPEXPORT void drop(const size_t theCount,
                           double& thePtsFront);

    /**
     * Function used to get current showed source format.
     * At this moment function used just for stereo/mono recognizing.
     */
    ST_LOCAL int getSrcFormat() {
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

    ST_CPPEXPORT int getSnapshot(StImage* theOutDataLeft,
                                 StImage* theOutDataRight,
                                 bool     theToForce = false);

        private:

    enum {
        SWAPONREADY_NOTHING = 0,
        SWAPONREADY_SWAPPED = 1,
        SWAPONREADY_WAITLIM = 2,
    };

        private:

    ST_CPPEXPORT int swapFBOnReady(StGLContext& theCtx);

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

    StCondition      myNewShotEvent;
    bool             myIsInUpdTexture; //!< private bools for plugin thread
    bool             myIsReadyToSwap;
    bool             myToCompress;     //!< release unused memory as fast as possible
    volatile bool    myHasStream;      //!< flag indicates that some stream connected to this queue

    StGLDeviceCaps   myDeviceCaps;     //!< device capabilities
    StHandle<StGLTextureUploadParams> myUploadParams; //!< texture streaming parameters

};

#endif //__StGLTextureQueue_h_
