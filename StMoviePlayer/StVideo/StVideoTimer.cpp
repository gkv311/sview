/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
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

#include "StVideoTimer.h"

#include <StThreads/StThread.h>

/**
 * Thread just call mainLoop() function.
 */
static SV_THREAD_FUNCTION refreshThread(void* videoTimer) {
    StVideoTimer* stVideoTimer = (StVideoTimer* )videoTimer;
    stVideoTimer->mainLoop();
    return SV_THREAD_RETURN 0;
}

StVideoTimer::StVideoTimer(const StHandle<StVideoQueue>& theVideo,
                           const StHandle<StAudioQueue>& theAudio,
                           const double                  theDelayVVFixedMs)
: myVideo(theVideo),
  myAudio(theAudio),
  myToQuitEv(false),
  myTimer(false),
  myTimerThrCurr(theDelayVVFixedMs),
  myTimerThrNext(theDelayVVFixedMs),
  myAudioPtsCurrSec(-1.0),
  myVideoPtsCurrSec(myVideo->getPts()),
  myVideoPtsNextSec(-1.0),
  myDelayTimer(0.0),
  myDiffVA(0.0),
  myDelayVAFixed(0),
  myDelayVV(0.0),
  myDelayVVAver(theDelayVVFixedMs),
  myDelayVVFixed(theDelayVVFixedMs),
  mySpeedFastSkip(3.0),
  mySpeedFast(1.5),
  mySpeedFastRev(1.0 / mySpeedFast),
  mySpeedSlow(0.4),
  mySpeedSlowRev(1.0 / mySpeedSlow),
  myIsBenchmark(false) {
    stMemZero(mySpeedDesc, sizeof(mySpeedDesc));
    myThread = new StThread(refreshThread, (void* )this, "StVideoTimer");
}

StVideoTimer::~StVideoTimer() {
    myToQuitEv.set();
    myThread->wait();
    myThread.nullify();
}

bool StVideoTimer::isQuitMessage() {
    for(;;) {
        if(myToQuitEv.check() && myVideo->isEmpty()) {
            return true;
        } else if(!myVideo->isPlaying()) {
            StThread::sleep(10);
            ///ST_DEBUG_LOG_AT("Not played!");
            myTimer.restart();
            myTimerThrNext = 0.0;
        } else {
            return false;
        }
    }
}

void StVideoTimer::mainLoop() {
    if(myVideo->getId() < 0) {
        return; // nothing to refresh
    }
    myVideo->setAClock(0.0);
    myTimer.restart();
    for(;;) {
        if(isQuitMessage()) {
            return;
        }

        if(myTimer.getElapsedTimeInMilliSec() >= myTimerThrNext) {
            // this is time we should show the next frame, call swap Front/Back here
            while(!myVideo->getTextureQueue()->stglSwapFB(1)) {
                if(isQuitMessage()) {
                    return;
                }
                StThread::sleep(1);
            }

            // store old timer threshold value to check diff at the end
            myTimerThrCurr = myTimerThrNext;

            // we got Video PTS for NEXT shown frame
            // so we need to compute time it will be shown
            myVideoPtsCurrSec = myVideoPtsNextSec; // just store for some conditions checks
            while(!myVideo->getTextureQueue()->popPTSNext(myVideoPtsNextSec)) {
                if(isQuitMessage()) {
                    return;
                }
                StThread::sleep(1);
            }

            myDelayVV = getDelayMsec(myVideoPtsNextSec, myVideoPtsCurrSec);
            if(myDelayVV > 0.0 && myDelayVV < 201.0) {
                myInfoLock.lock();
                myDelayVVAver = myDelayVV;
                myInfoLock.unlock();
            }
            if(myVideoPtsNextSec >= 0.0) {
                // try Audio to Video sync
                if(myAudio->getId() >= 0) {
                    // we got current Audio PTS value
                    myAudioPtsCurrSec = myAudio->getPts();
                    if(myAudioPtsCurrSec > 0.0) {
                        myVideo->setAClock(myAudioPtsCurrSec);
                        myDiffVA = getDelayMsec(myVideoPtsNextSec, myAudioPtsCurrSec);
                        myDelayTimer = myDiffVA - double(myDelayVAFixed);
                    }
                } else if(myVideoPtsCurrSec < 0.0) {
                    // empty video queue or first frame
                    myDelayTimer = myDelayVVFixed;
                } else {
                    // increase timer threshold to delay between frames
                    myDelayTimer = myDelayVV;
                }

                // fix values out from range
                if(mySpeedSlow * myDelayTimer > myDelayVVAver) {
                    myDelayTimer = mySpeedSlowRev * myDelayVVAver;
                } else if(mySpeedFastSkip * myDelayTimer < myDelayVVAver) {
                    //myVideo->getTextureQueue()->drop(2);
                    myVideo->getTextureQueue()->drop(1);
                    myDelayTimer = mySpeedFastRev * myDelayVVAver;
                } else if(mySpeedFast * myDelayTimer < myDelayVVAver) {
                    myDelayTimer = mySpeedFastRev * myDelayVVAver;
                } else {
                    //ST_DEBUG_LOG(getSpeedText() + "|  normal  |myDelayTimer= " + myDelayTimer + ", myDelayVV= " + myDelayVV);
                }
            } else {
                // fixed FPS
                myDelayTimer = myDelayVVFixed;
            }
            myTimerThrNext = myTimerThrCurr + myDelayTimer;
            if(myIsBenchmark) {
                myTimerThrNext = 0.0;
            }
        }
        StThread::sleep(1);
    }
}
