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

#ifndef __StVideoTimer_h_
#define __StVideoTimer_h_

#include "StVideoQueue.h"   // video queue class
#include "StAudioQueue.h"   // audio queue class

/**
 * This class represents video refresher
 * and Audio to Video sync.
 */
class StVideoTimer {

        public:

    /**
     * Constructor.
     * @param theVideo master video stream to trigger frame update
     * @param theAudio audio stream to synchronize from
     * @param theDelayVVFixedMs default video frame delay for streams with fixed FPS
     */
    ST_LOCAL StVideoTimer(const StHandle<StVideoQueue>& theVideo,
                          const StHandle<StAudioQueue>& theAudio,
                          const double                  theDelayVVFixedMs = 40.0);

    /**
     * Destructor.
     */
    ST_LOCAL ~StVideoTimer();

    /**
     * Ignore sync rules and perform swap when ready.
     */
    ST_LOCAL void setBenchmark(const bool theToPerformBenchmark) {
        myIsBenchmark = theToPerformBenchmark;
    }

    /**
     * Setup video/audio delay.
     */
    ST_LOCAL void setAudioDelay(const int theDelayMSec) {
        myDelayVAFixed = theDelayMSec;
    }

    /*ST_LOCAL double getSpeed() const {
        // TODO (Kirill Gavrilov#5#) not thread-safe
        return myDelayVVAver / myDelayTimer;
    }

    ST_LOCAL StString getSpeedText(const double& theValue) {
        sprintf(mySpeedDesc, "x%.2f", theValue);
        return StString(mySpeedDesc);
    }

    ST_LOCAL StString getSpeedText() {
        return getSpeedText(getSpeed());
    }*/

    /**
     * @return average FPS required to play video stream
     */
    ST_LOCAL double getAverFps() const {
        myInfoLock.lock();
        double anAver = myDelayVVAver;
        myInfoLock.unlock();
        return 1000.0 / anAver;
    }

    /**
     * Main refresher loop function.
     * Should be run on dedicated thread.
     */
    ST_LOCAL void mainLoop();

        private:

    ST_LOCAL static double getDelayMsec(const double& theNextSec,
                                        const double& theCurrSec) {
        return (theNextSec - theCurrSec) * 1000.0;
    }

    ST_LOCAL bool isQuitMessage();

        private:

    StHandle<StThread>     myThread;          //!< timer loop thread
    StHandle<StVideoQueue> myVideo;           //!< video queue to sync
    StHandle<StAudioQueue> myAudio;           //!< audio queue to sync from
    mutable StMutex        myInfoLock;        //!< lock to retrieve information from other threads
    StCondition            myToQuitEv;        //!< thread exit event
    StTimer                myTimer;           //!< timer to refresh frames

    double                 myTimerThrCurr;    //!< current timer threshold (timer expired) (in milliseconds)
    double                 myTimerThrNext;    //!< timer threshold to show next Video frame (in milliseconds)

    double                 myAudioPtsCurrSec; //!< real time Audio PTS value (in seconds)
    double                 myVideoPtsCurrSec; //!< current Video frame PTS value (in seconds)
    double                 myVideoPtsNextSec; //!< next Video frame PTS value (in seconds)

    double                 myDelayTimer;      //!< timer delay (in milliseconds)
    double                 myDiffVA;          //!< Audio to Video PTS diff (in milliseconds)
    volatile int           myDelayVAFixed;    //!< Video delayed from Audio on this value (in milliseconds)
    double                 myDelayVV;         //!< real frame's delay (in milliseconds)
    double                 myDelayVVAver;
    double                 myDelayVVFixed;    //!< fixed (for constant FPS) frame's delay (in milliseconds)

    double                 mySpeedFastSkip;   //!< video playback too FAST, so we speed down and SKIP some ready frames
    double                 mySpeedFast;       //!< video playback too FAST, so we speed down to this value
    double                 mySpeedFastRev;
    double                 mySpeedSlow;       //!< video playback too FAST, so we speed up to this value
    double                 mySpeedSlowRev;

    char                   mySpeedDesc[256];

    bool                   myIsBenchmark;

};

#endif // __StVideoTimer_h_
