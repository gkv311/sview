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
                           const double delayVVFixedMs)
: myVideo(theVideo),
  myAudio(theAudio),
  evDoEndLoop(false),
  refreshTimer(false),
  timerThrCurr(delayVVFixedMs),
  timerThrNext(delayVVFixedMs),
  aPtsCurrSec(-1.0),
  vPtsCurrSec(myVideo->getPts()),
  vPtsNextSec(-1.0),
  delayToSwap(2.0),
  delayTimer(0.0),
  diffVA(0.0),
  delayVAFixed(0.0),
  delayVV(0.0),
  delayVVAver(delayVVFixedMs),
  delayVVFixed(delayVVFixedMs),
  speedFastSkip(3.0),
  speedFast(1.5),
  speedFastRev(1.0 / speedFast),
  speedSlow(0.4),
  speedSlowRev(1.0 / speedSlow),
  isBenchmark(false) {
    //
    stMemSet(speedDesc, 0, sizeof(speedDesc));
    myThread = new StThread(refreshThread, (void* )this);
}

StVideoTimer::~StVideoTimer() {
    evDoEndLoop.set();
    myThread->wait();
    myThread.nullify();
}

bool StVideoTimer::isQuitMessage() {
    for(;;) {
        if(evDoEndLoop.check() && myVideo->isEmpty()) {
            return true;
        } else if(!myVideo->isPlaying()) {
            StThread::sleep(10);
            ///ST_DEBUG_LOG_AT("Not played!");
            refreshTimer.restart();
            timerThrNext = 0.0;
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
    refreshTimer.restart();
    for(;;) {
        if(isQuitMessage()) {
            return;
        }
        // TODO (Kirill Gavrilov#3#) analize delayToSwap value
        // we have parasite freezes if video FPS ~25 and display FPS ~60Hz
        // video played good if video FPS ~30
        if(refreshTimer.getElapsedTimeInMilliSec() >= timerThrNext) {
            // this is time we should show the next frame, call swap Front/Back here
            while(!myVideo->getTextureQueue()->stglSwapFB(1)) {
                if(isQuitMessage()) {
                    return;
                }
                StThread::sleep(1);
            }

            // store old timer threshold value to check diff at the end
            timerThrCurr = timerThrNext;

            // we got Video PTS for NEXT shown frame
            // so we need to compute time it will be shown
            vPtsCurrSec = vPtsNextSec; // just store for some conditions checks
            while(!myVideo->getTextureQueue()->popPTSNext(vPtsNextSec)) {
                if(isQuitMessage()) {
                    return;
                }
                StThread::sleep(1);
                ///ST_DEBUG_LOG("waittt2");
            }

            delayVV = getDelayMsec(vPtsNextSec, vPtsCurrSec);
            if(delayVV > 0.0 && delayVV < 201.0) {
                myInfoLock.lock();
                delayVVAver = delayVV;
                myInfoLock.unlock();
            }
            if(vPtsNextSec >= 0.0) {
                // try Audio to Video sync
                if(myAudio->getId() >= 0) {
                    // we got current Audio PTS value
                    aPtsCurrSec = myAudio->getPts();
                    if(aPtsCurrSec > 0.0) {
                        myVideo->setAClock(aPtsCurrSec);
                        diffVA = getDelayMsec(vPtsNextSec, aPtsCurrSec);
                        delayTimer = diffVA + delayVAFixed;
                    }
                } else if(vPtsCurrSec < 0.0) {
                    // empty video queue or first frame
                    delayTimer = delayVVFixed;
                } else {
                    // increase timer threshold to delay between frames
                    delayTimer = delayVV;
                }

                // fix values out from range
                if(speedSlow * delayTimer > delayVVAver) {
                    ///ST_DEBUG_LOG(getSpeedText(speedSlow) + "| too SLOW |" + getSpeedText() + ", delayTimer= " + delayTimer + ", delayVV= " + delayVV);
                    delayTimer = speedSlowRev * delayVVAver;
                } else if(speedFastSkip * delayTimer < delayVVAver) {
                    ///ST_DEBUG_LOG(getSpeedText(speedFast) + "|very FAST |" + getSpeedText() + ", delayTimer= " + delayTimer + ", delayVV= " + delayVV);
                    //myVideo->getTextureQueue()->drop(2);
                    myVideo->getTextureQueue()->drop(1);
                    delayTimer = speedFastRev * delayVVAver;
                } else if(speedFast * delayTimer < delayVVAver) {
                    ///ST_DEBUG_LOG(getSpeedText(speedFast) + "| too FAST |" + getSpeedText() + ", delayTimer= " + delayTimer + ", delayVV= " + delayVV);
                    delayTimer = speedFastRev * delayVVAver;
                } else {
                    ///ST_DEBUG_LOG(getSpeedText() + "|  normal  |delayTimer= " + delayTimer + ", delayVV= " + delayVV);
                }
                ///ST_DEBUG_LOG("aPTS= " + aPtsCurrSec + ", vPTS= " + vPtsNextSec + ", diffVA= " + diffVA);
            } else {
                // fixed FPS
                delayTimer = delayVVFixed;
            }
            timerThrNext = timerThrCurr + delayTimer;
            if(isBenchmark) {
                timerThrNext = 0.0;
            }
        }
        StThread::sleep(1);
    }
}
