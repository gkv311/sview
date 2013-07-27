/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLStereo/StGLTextureQueue.h>

StGLTextureQueue::StGLTextureQueue(const size_t theQueueSizeMax)
: myDataFront(NULL),
  myDataSnap(NULL),
  myDataBack(NULL),
  myQueueSize(0),
  myQueueSizeMax(theQueueSizeMax),
  mySwapFBCount(0),
  myCurrSrcFormat(ST_V_SRC_MONO),
  myCurrPts(0.0),
  myNewShotEvent(false),
  myIsInUpdTexture(false),
  myIsReadyToSwap(false),
  myToCompress(false),
  myHasStream(false) {
    ST_DEBUG_ASSERT(myQueueSizeMax >= 2);

    // we create 'empty' queue
    myDataFront = new StGLTextureData();
    StGLTextureData* iter = myDataFront;
    for(size_t i = 1; i < myQueueSizeMax; ++i) {
        iter->setNext(new StGLTextureData());
        iter = iter->getNext();
    }
    iter->setNext(myDataFront); // data in loop
    myDataBack = myDataFront;
}

StGLTextureQueue::~StGLTextureQueue() {
    for(size_t anIter = 0; anIter < myQueueSizeMax; ++anIter) {
        StGLTextureData* aRemItem = myDataFront;
        myDataFront = myDataFront->getNext();
        delete aRemItem;
    }
}

void StGLTextureQueue::setCompressMemory(const bool theToCompress) {
    myToCompress = theToCompress;
}

// this function called ONLY from image thread
bool StGLTextureQueue::push(const StImage&     theSrcDataLeft,
                            const StImage&     theSrcDataRight,
                            const StHandle<StStereoParams>& theStParams,
                            const StFormatEnum theSrcFormat,
                            const double       theSrcPTS) {
    if(isFull()) {
        return false;
    }

    myMutexPush.lock();
    myDataBack = isEmpty() ? myDataFront : myDataBack->getNext();

    myDataBack->updateData(theSrcDataLeft,
                           theSrcDataRight,
                           theStParams,
                           theSrcFormat,
                           theSrcPTS);
    myMutexSrcFormat.lock();
        myCurrSrcFormat = myDataBack->getSourceFormat();
    myMutexSrcFormat.unlock();

    myMutexSize.lock();
        ++myQueueSize;
    myMutexSize.unlock();
    myMutexPush.unlock();
    return true;
}

int StGLTextureQueue::swapFBOnReady(StGLContext& theCtx) {
    if(!myIsReadyToSwap) {
        return SWAPONREADY_NOTHING;
    }

    mySwapFBMutex.lock();
    if(mySwapFBCount != 0) {
        myIsReadyToSwap = false;
        --mySwapFBCount;
        mySwapFBMutex.unlock();

        myQTexture.swapFB();
        if(myToCompress) {
            myQTexture.getBack(StGLQuadTexture::LEFT_TEXTURE ).release(theCtx);
            myQTexture.getBack(StGLQuadTexture::RIGHT_TEXTURE).release(theCtx);
        }

        myMeterMutex.lock();
            ++myFPSMeter;
        myMeterMutex.unlock();
        return SWAPONREADY_SWAPPED;
    } else {
        mySwapFBMutex.unlock();
        return SWAPONREADY_WAITLIM;
    }
}

// this function called ONLY from plugin thread
bool StGLTextureQueue::stglUpdateStTextures(StGLContext& theCtx) {
    int aSwapState = swapFBOnReady(theCtx);
    if(aSwapState == SWAPONREADY_WAITLIM) {
        return false;
    }

    if(!myMutexPop.tryLock()) {
        return aSwapState == SWAPONREADY_SWAPPED;
    }

    // do we already in update cycle?
    if(!myIsInUpdTexture) {
        // check event from video thread
        if(!isEmpty()) {
            myIsInUpdTexture = true;
        }
    } else if(isEmpty()) {
        // if we in texture update sequence - check queue not emptied!
        myIsInUpdTexture = false;
    }

    // still nothing to update? so return
    if(!myIsInUpdTexture) {
        myMutexPop.unlock();
        return aSwapState == SWAPONREADY_SWAPPED;
    }

    if(myDataFront->fillTexture(theCtx, myQTexture)) {
        myIsReadyToSwap = true;
        myMutexSize.lock();
            myCurrPts   = myDataFront->getPTS();
            myDataSnap  = myDataFront; myNewShotEvent.set();
            if(myToCompress) {
                myDataFront->reset();
            }
            myDataFront = myDataFront->getNext();
            ST_DEBUG_ASSERT(myQueueSize != 0); // critical error!
            --myQueueSize;
        myMutexSize.unlock();
        myIsInUpdTexture = false;
    }
    myMutexPop.unlock();

    // try early swap
    const bool isAlreadySwapped = (aSwapState == SWAPONREADY_SWAPPED);
    aSwapState = swapFBOnReady(theCtx);
    return (aSwapState == SWAPONREADY_SWAPPED || isAlreadySwapped);
}

void StGLTextureQueue::clear() {
    myMutexPop.lock();
    myMutexPush.lock();
    myMutexSize.lock();
    mySwapFBMutex.lock();
        // decrease StStereoSource counters
        StGLTextureData* anIterFront = myDataFront;
        for(size_t i = 0; i < myQueueSize; ++i, anIterFront = anIterFront->getNext()) {
            anIterFront->resetStParams();
        }
        // reset queue
        myQueueSize     = 0;
        myDataBack      = myDataFront;
        myDataSnap      = NULL;
        mySwapFBCount   = 0;
        myIsReadyToSwap = false; // invalidate currently uploaded image in back buffer
        // empty texture update sequence
        myIsInUpdTexture = false;
    mySwapFBMutex.unlock();
    myMutexSize.unlock();
    myMutexPush.unlock();
    myMutexPop.unlock();
}

void StGLTextureQueue::drop(const size_t theCount) {
    myMutexPop.lock();
    myMutexPush.lock();
    myMutexSize.lock();
        if(myQueueSize < 2) {
            // to small queue
            myMutexSize.unlock();
            myMutexPush.unlock();
            myMutexPop.unlock();
            return;
        }
        size_t decr = (theCount < myQueueSize) ? theCount : (myQueueSize - 1);

        // decrease StStereoSource counters
        for(size_t i = 0; i < decr; ++i, myDataFront = myDataFront->getNext()) {
            myDataFront->resetStParams();
        }
        // reset queue
        myQueueSize -= decr;
        // empty texture update sequence
        myIsInUpdTexture = false;
    myMutexSize.unlock();
    myMutexPush.unlock();
    myMutexPop.unlock();
}

int StGLTextureQueue::getSnapshot(StImage* theOutDataLeft,
                                  StImage* theOutDataRight,
                                  bool     theToForce) {
    if(!myNewShotEvent.check() && !theToForce) {
        return SNAPSHOT_NO_NEW;
    }
    myMutexPop.lock();
    if(myDataSnap == NULL) {
        myMutexPop.unlock();
        return SNAPSHOT_NO_NEW;
    }
    myDataSnap->getCopy(theOutDataLeft, theOutDataRight);
    myNewShotEvent.reset();
    myMutexPop.unlock();
    return SNAPSHOT_SUCCESS;
}
