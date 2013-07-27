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
 * and Audio 2 Video sync.
 */
class StVideoTimer {

        public:

    ST_LOCAL StVideoTimer(const StHandle<StVideoQueue>& theVideo,
                          const StHandle<StAudioQueue>& theAudio,
                          const double delayVVFixedMs = 40.0);

    ST_LOCAL ~StVideoTimer();

    /**
     * Ignore sync rules and perform swap when ready.
     */
    ST_LOCAL void setBenchmark(bool toPerformBenchmark) {
        isBenchmark = toPerformBenchmark;
    }

    ST_LOCAL double getSpeed() const {
        // TODO (Kirill Gavrilov#5#) not thread-safe
        return delayVVAver / delayTimer;
    }

    ST_LOCAL StString getSpeedText(const double& value) {
        sprintf(speedDesc, "x%.2f", value);
        return StString(speedDesc);
    }

    ST_LOCAL StString getSpeedText() {
        return getSpeedText(getSpeed());
    }

    ST_LOCAL double getAverFps() const {
        myInfoLock.lock();
        double anAver = delayVVAver;
        myInfoLock.unlock();
        return 1000.0 / anAver;
    }

    /**
     * Main refresher loop function.
     * Should be run on dedicated thread.
     */
    ST_LOCAL void mainLoop();

        private:

    ST_LOCAL static double getDelayMsec(const double& nextSec, const double& currSec) {
        return (nextSec - currSec) * 1000.0;
    }

    ST_LOCAL bool isQuitMessage();

        private:

    StHandle<StThread>     myThread;   //!< timer loop thread
    StHandle<StVideoQueue> myVideo;    //!< video queue to sync
    StHandle<StAudioQueue> myAudio;    //!< audio queue to sync from
    mutable StMutex        myInfoLock;
    StCondition evDoEndLoop; // thread exit event
    StTimer refreshTimer; // timer to refresh frames

    double  timerThrCurr; // current timer threshold (timer expired) (in milliseconds)
    double  timerThrNext; // timer threshold to show next Video frame (in milliseconds)

    double   aPtsCurrSec; // real time Audio PTS value (in seconds)
    double   vPtsCurrSec; // current Video frame PTS value (in seconds)
    double   vPtsNextSec; // next Video frame PTS value (in seconds)

    double   delayToSwap; // special delay to send swap signal early (to prevent VSync freezes) (in milliseconds)
    double    delayTimer; // timer delay (in milliseconds)
    double        diffVA; // Audio to Video PTS diff (in milliseconds)
    double  delayVAFixed; // Video delayed from Audio on this value (in milliseconds)
    double       delayVV; // real frame's delay (in milliseconds)
    double   delayVVAver; //
    double  delayVVFixed; // fixed (for constant FPS) frame's delay (in milliseconds)

    double speedFastSkip; // video playback too FAST, so we speed down and SKIP some ready frames
    double     speedFast; // video playback too FAST, so we speed down to this value
    double  speedFastRev;
    double     speedSlow; // video playback too FAST, so we speed up to this value
    double  speedSlowRev;

    char  speedDesc[256];

    bool     isBenchmark;

};

#endif //__StVideoTimer_h_
